/******************************************************************************************************************************/
/* Include/Memcache.h        Déclaration structure internes pour le cache memcached                                           */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                18.11.2026 19:40:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Memcache.h
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

#ifndef _MEMCACHE_H_
 #define _MEMCACHE_H_

 #define MEMCACHE_KEY_SIZE   192                                     /* domain_uuid + génération + sha256 hexa (64 car) */

/*************************************************** Définitions des prototypes ***********************************************/
 extern gboolean MEMCACHE_Init ( void );
 extern void MEMCACHE_End ( void );
 extern gboolean MEMCACHE_Build_key ( struct DOMAIN *domain, gchar *requete, gchar *cache_key, gsize cache_key_size );
 extern gchar *MEMCACHE_Get ( struct DOMAIN *domain, gchar *cache_key );
 extern void MEMCACHE_Set ( struct DOMAIN *domain, gchar *cache_key, gchar *value, gint retention );
 extern void DB_Cache_invalidate ( struct DOMAIN *domain );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
