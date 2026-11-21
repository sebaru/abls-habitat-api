/******************************************************************************************************************************/
/* memcache.c          Gestion du pool de clients memcached partagé par tous les domaines                                     */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                18.11.2026 19:40:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * memcache.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <glib.h>
 #include <string.h>
 #include <stdlib.h>

/**************************************************** Chargement des prototypes ***********************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* MEMCACHE_Init: Crée le pool global de clients memcached selon la configuration                                             */
/* Entrée: néant                                                                                                              */
/* Sortie: FALSE si la configuration est invalide. Un cache désactivé (memcached_options vide) n'est pas une erreur           */
/******************************************************************************************************************************/
 gboolean MEMCACHE_Init ( void )
  { Global.memcached_pool = NULL;

    gchar *options = Json_get_string ( Global.config, "memcached_options" );
    if (!options || !*options)
     { Info ( __func__, "memcache", "master", LOG_NOTICE, "memcached_options is empty: DB Cache disabled" );
       return(TRUE);
     }

    gint pool_min = Json_get_int ( Global.config, "memcached_pool_min" );
    gint pool_max = Json_get_int ( Global.config, "memcached_pool_max" );
    if (pool_min < 1 || pool_max < pool_min)
     { Info ( __func__, "memcache", "master", LOG_CRIT, "Invalid pool size: memcached_pool_min=%d, memcached_pool_max=%d "
                                                        "(expected 1 <= min <= max)", pool_min, pool_max );
       return(FALSE);
     }

    memcached_st *master = memcached ( options, strlen(options) );
    if (!master)
     { Info ( __func__, "memcache", "master", LOG_CRIT, "Invalid memcached_options '%s'", options );
       return(FALSE);
     }

    Global.memcached_pool = memcached_pool_create ( master, pool_min, pool_max );
    if (!Global.memcached_pool)
     { Info ( __func__, "memcache", "master", LOG_CRIT, "Unable to create memcached pool (min=%d, max=%d)", pool_min, pool_max );
       memcached_free ( master );
       return(FALSE);
     }

    Info ( __func__, "memcache", "master", LOG_NOTICE, "DB Cache enabled with options '%s', pool min=%d, max=%d",
           options, pool_min, pool_max );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* MEMCACHE_End: Détruit le pool global de clients memcached                                                                  */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MEMCACHE_End ( void )
  { if (!Global.memcached_pool) return;
    memcached_st *master = memcached_pool_destroy ( Global.memcached_pool );      /* Le master de pool_create reste à libérer */
    if (master) memcached_free ( master );
    Global.memcached_pool = NULL;
    Info ( __func__, "memcache", "master", LOG_INFO, "DB Cache pool destroyed" );
  }
/******************************************************************************************************************************/
/* MEMCACHE_Fetch: Emprunte un client memcached au pool, sans attente                                                         */
/* Entrée: le domaine (pour les logs et compteurs)                                                                            */
/* Sortie: le client, ou NULL si cache désactivé, pool saturé ou erreur. Le client doit être rendu par MEMCACHE_Release      */
/******************************************************************************************************************************/
 static memcached_st *MEMCACHE_Fetch ( struct DOMAIN *domain )
  { if (!Global.memcached_pool) return(NULL);

    memcached_return_t rc;
    memcached_st *memc = memcached_pool_fetch ( Global.memcached_pool, NULL, &rc );   /* NULL = non bloquant: repli SQL */
    if (!memc)
     { g_atomic_int_inc ( &domain->cache_errors );
       if (rc == MEMCACHED_NOTFOUND)
        { Info ( __func__, "memcache", domain->uuid, LOG_DEBUG, "Memcached pool exhausted, falling back to database" ); }
       else Info ( __func__, "memcache", domain->uuid, LOG_ERR, "Memcached pool fetch error -> '%s'", memcached_strerror ( NULL, rc ) );
     }
    return(memc);
  }
/******************************************************************************************************************************/
/* MEMCACHE_Release: Rend un client memcached au pool                                                                         */
/* Entrée: le domaine (pour les logs), le client                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void MEMCACHE_Release ( struct DOMAIN *domain, memcached_st *memc )
  { memcached_return_t rc = memcached_pool_release ( Global.memcached_pool, memc );
    if (rc != MEMCACHED_SUCCESS)
     { Info ( __func__, "memcache", domain->uuid, LOG_ERR, "Memcached pool release error -> '%s'", memcached_strerror ( NULL, rc ) ); }
  }
/******************************************************************************************************************************/
/* MEMCACHE_Build_key: Construit la clef de cache d'une requete pour la génération courante du domaine                        */
/* Entrée: le domaine, la requete, le buffer de sortie et sa taille                                                           */
/* Sortie: FALSE si le cache est désactivé ou si la clef ne tient pas dans le buffer                                          */
/******************************************************************************************************************************/
 gboolean MEMCACHE_Build_key ( struct DOMAIN *domain, gchar *requete, gchar *cache_key, gsize cache_key_size )
  { if (!Global.memcached_pool || !domain || !requete) return(FALSE);

    guchar cache_key_bin[SHA256_DIGEST_LENGTH];
    gchar chaine[2*SHA256_DIGEST_LENGTH+1];
    SHA256( (guchar *)requete, strlen(requete), cache_key_bin );
    for (gint cpt = 0; cpt < SHA256_DIGEST_LENGTH; cpt++)
     { g_snprintf( chaine + (cpt * 2), 3, "%02X", cache_key_bin[cpt] ); }

    gint taille = g_snprintf( cache_key, cache_key_size, "%s:g%llu:sha256:%s",
                              Json_get_string ( domain->config, "domain_uuid" ),
                              (unsigned long long)__atomic_load_n ( &domain->cache_generation, __ATOMIC_RELAXED ), chaine );
    if (taille < 0 || (gsize)taille >= cache_key_size)
     { Info ( __func__, "memcache", domain->uuid, LOG_ERR, "Cache key too long, not using DB Cache" );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* MEMCACHE_Get: Lit une valeur dans le cache                                                                                 */
/* Entrée: le domaine, la clef                                                                                                */
/* Sortie: la valeur (chaine terminée par \0, à libérer par g_free), ou NULL si absente/erreur                                */
/******************************************************************************************************************************/
 gchar *MEMCACHE_Get ( struct DOMAIN *domain, gchar *cache_key )
  { memcached_st *memc = MEMCACHE_Fetch ( domain );
    if (!memc) return(NULL);

    size_t value_length = 0;
    memcached_return_t hit;
    gchar *value = memcached_get( memc, cache_key, strlen(cache_key), &value_length, NULL, &hit );
    gchar *result = NULL;
    if (hit == MEMCACHED_SUCCESS && value)
     { result = g_strndup ( value, value_length ); }                         /* La valeur memcached n'est pas terminée par \0 */
    else if (hit == MEMCACHED_NOTFOUND)
     { g_atomic_int_inc ( &domain->cache_misses ); }
    else
     { g_atomic_int_inc ( &domain->cache_errors );
       Info ( __func__, "memcache", domain->uuid, LOG_ERR, "DB CACHE Read Error -> '%s'", memcached_strerror( memc, hit ) );
     }
    free ( value );
    MEMCACHE_Release ( domain, memc );
    return(result);
  }
/******************************************************************************************************************************/
/* MEMCACHE_Set: Ecrit une valeur dans le cache                                                                               */
/* Entrée: le domaine, la clef, la valeur et sa durée de rétention en secondes                                                */
/* Sortie: néant. Un échec d'écriture n'est pas bloquant                                                                      */
/******************************************************************************************************************************/
 void MEMCACHE_Set ( struct DOMAIN *domain, gchar *cache_key, gchar *value, gint retention )
  { if (!value) return;
    memcached_st *memc = MEMCACHE_Fetch ( domain );
    if (!memc) return;

    memcached_return_t stored = memcached_set( memc, cache_key, strlen(cache_key), value, strlen(value), retention, 0 );
    if (stored != MEMCACHED_SUCCESS)
     { g_atomic_int_inc ( &domain->cache_errors );
       Info ( __func__, "memcache", domain->uuid, LOG_ERR, "DB CACHE Write Error -> '%s'", memcached_strerror( memc, stored ) );
     }
    MEMCACHE_Release ( domain, memc );
  }
/******************************************************************************************************************************/
/* DB_Cache_invalidate: Invalide l'ensemble du cache DB du domaine                                                            */
/* Entrée: le domaine                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DB_Cache_invalidate ( struct DOMAIN *domain )
  { if (!domain) return;

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gchar gen_key[128];
    g_snprintf ( gen_key, sizeof(gen_key), "%s:cache_generation", domain_uuid );

    gboolean invalidated = FALSE;
    memcached_st *memc = MEMCACHE_Fetch ( domain );
    if (memc)
     { uint64_t generation;                       /* La génération est partagée en cache pour survivre aux redémarrages d'API */
       if ( memcached_increment_with_initial ( memc, gen_key, strlen(gen_key), 1, 1, 0, &generation ) == MEMCACHED_SUCCESS )
        { guint64 current = __atomic_load_n ( &domain->cache_generation, __ATOMIC_RELAXED );
          guint64 next;
          do next = MAX ( current + 1, generation );                       /* Avance toujours, même si le local est en avance */
          while (!__atomic_compare_exchange_n ( &domain->cache_generation, &current, next, TRUE,
                                                __ATOMIC_RELAXED, __ATOMIC_RELAXED ));
          invalidated = TRUE;
        }
       MEMCACHE_Release ( domain, memc );
     }
                                                           /* si pb avec memcached, on avance quand même la génération locale */
    if (!invalidated) __atomic_add_fetch ( &domain->cache_generation, 1, __ATOMIC_RELAXED );

    Info ( __func__, "memcache", domain->uuid, LOG_DEBUG, "DB CACHE invalidated, generation is now '%llu'",
           (unsigned long long)__atomic_load_n ( &domain->cache_generation, __ATOMIC_RELAXED ) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
