/******************************************************************************************************************************/
/* TraductionDLS/Interp.c          Interpretation du langage DLS                                                              */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                      dim 05 avr 2009 12:47:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Interp.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <locale.h>

 /************************************************** Prototypes de fonctions ***************************************************/
 #include "lignes.h"
 #include "Http.h"
 #include "Dls_trad.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* New_chaine: Alloue une certaine quantité de mémoire pour utiliser des chaines de caractères                                */
/* Entrées: la longueur souhaitée                                                                                             */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 char *New_chaine( int longueur )
  { char *chaine;
    chaine = g_try_malloc0( longueur+1 );
    if (!chaine) { return(NULL); }
    return(chaine);
  }
/******************************************************************************************************************************/
/* Emettre: Met a jour le fichier temporaire en code intermédiaire                                                            */
/* Entrées: la ligne d'instruction à mettre                                                                                   */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Emettre( void *scan_instance, char *chaine )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gint taille = strlen(chaine);
    if ( Dls_scanner->buffer_used + taille >= Dls_scanner->buffer_size )
     { gint new_taille = Dls_scanner->buffer_used + taille + 1;
       Info_new( __func__, LOG_DEBUG, Dls_scanner->domain, "Buffer too small, trying to expand it to %d)", new_taille );
       gchar *new_Buffer = g_try_realloc( Dls_scanner->Buffer, new_taille );
       if (!new_Buffer)
        { Info_new( __func__, LOG_ERR, Dls_scanner->domain, "Fail to expand buffer. skipping" );
          return;
        }
       Dls_scanner->Buffer      = new_Buffer;
       Dls_scanner->buffer_size = new_taille;
       Info_new( __func__, LOG_DEBUG, Dls_scanner->domain, "Buffer expanded to %d bytes", Dls_scanner->buffer_size );
     }
    Info_new( __func__, LOG_DEBUG, Dls_scanner->domain,
              "Ligne %d : %s", DlsScanner_get_lineno(scan_instance), chaine );
    memcpy ( Dls_scanner->Buffer + Dls_scanner->buffer_used, chaine, taille+1 );                 /* Recopie du bout de buffer */
    Dls_scanner->buffer_used += taille;
  }
/******************************************************************************************************************************/
/* Dls_check_mode_VISUEL: Vérifie si la forme en parametre est disponible dans le mode en parametre                           */
/* Entrée: la forme, le mode                                                                                                  */
/* Sortie: TRUE si le mode existe (ou pas necessaire au visuel), FALSE sinon                                                  */
/******************************************************************************************************************************/
 static gboolean Dls_check_mode_VISUEL ( struct DLS_TRAD *Dls_scanner, gchar *forme, gchar *mode )
  { if (!Dls_scanner) return(FALSE);
    if (!forme) return(FALSE);
    if (!mode)  return(FALSE);

    GSList *cache = Dls_scanner->Visuel_check_cache;
    while ( cache )
     { gchar *cache_forme = Json_get_string ( cache->data, "forme" );
       gchar *cache_mode  = Json_get_string ( cache->data, "mode" );
       if (!strcmp ( cache_forme, forme ) && !strcmp( cache_mode, mode ) )
        { return ( Json_get_bool ( cache->data, "result" ) ); }
       cache = g_slist_next ( cache );
     }

    JsonNode *RootNode = Json_node_create();
    if ( !RootNode ) return(FALSE);
    Json_node_add_string ( RootNode, "forme", forme );
    Json_node_add_string ( RootNode, "mode",  mode );

    gboolean retour   = FALSE;
    gchar *mode_safe  = Normaliser_chaine ( mode );
    gchar *forme_safe = Normaliser_chaine ( forme );

    if (mode_safe && forme_safe)                            /* Chargement des parametres en base de données pour vérification */
     { DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL,
                 "SELECT icon_id, controle FROM icons WHERE forme='%s'", forme_safe );
       if ( Json_has_member ( RootNode, "icon_id" ) )
        { gchar *controle = Json_get_string ( RootNode, "controle" );
          if ( strcmp ( controle, "by_mode" ) && strcmp ( controle, "by_mode_color" ) )
           { retour = TRUE; }                                                        /* Si pas de controle par mode, alors OK */
          else
           { DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL,
                 "SELECT icon_mode_id FROM icons_modes "
                 "WHERE forme='%s' AND mode='%s'", forme_safe, mode_safe );
             if ( Json_has_member ( RootNode, "icon_mode_id" ) )
              { retour = TRUE; }                                    /* Si controle by mode ou mode_color et trouvé -> mode OK */
           }
        }
      }

    if (mode_safe)  g_free(mode_safe);
    if (forme_safe) g_free(forme_safe);
    Dls_scanner->Visuel_check_cache = g_slist_prepend ( Dls_scanner->Visuel_check_cache, RootNode );         /* Mise en cache */
    Json_node_add_bool ( RootNode, "result", retour );
    return(retour);
  }
/******************************************************************************************************************************/
/* DlsScanner_error: Appellé par le scanner en cas d'erreur de syntaxe (et non une erreur de grammaire !)                     */
/* Entrée : la chaine source de l'erreur de syntaxe                                                                           */
/* Sortie : appel de la fonction Emettre_erreur_new en backend                                                                */
/******************************************************************************************************************************/
 int DlsScanner_error ( void *scan_instance, char *s )
  { Emettre_erreur_new ( scan_instance, "%s", s );
    return(0);
  }
/******************************************************************************************************************************/
/* Emettre_erreur_new: collecte des erreurs de traduction D.L.S                                                               */
/* Entrée: le numéro de ligne, le format et les paramètres associés                                                           */
/******************************************************************************************************************************/
 void Emettre_erreur_new ( void *scan_instance, gchar *format, ... )
  { static gchar *too_many="Too many events. Limiting output...\n";
    gchar log[256], chaine[256];
    va_list ap;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    if (Dls_scanner->nbr_erreur<15)
     { va_start( ap, format );
       g_vsnprintf( chaine, sizeof(chaine), format, ap );
       va_end ( ap );
       g_snprintf( log, sizeof(log), "Ligne %d: %s\n", DlsScanner_get_lineno(scan_instance), chaine );
     }
    else if (Dls_scanner->nbr_erreur==15)
     { g_snprintf( log, sizeof(log), "%s\n", too_many ); }
    else return;

    gint new_taille = strlen(Dls_scanner->Error) + strlen(log) + 1;
    Dls_scanner->Error = g_try_realloc ( Dls_scanner->Error, new_taille );
    if (!Dls_scanner->Error) return;
    g_strlcat ( Dls_scanner->Error, log, new_taille );

    Info_new( __func__, LOG_ERR, Dls_scanner->domain, "'%s': %s", Json_get_string ( Dls_scanner->PluginNode, "tech_id" ), log );
    Dls_scanner->nbr_erreur++;
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct OPTION *New_option( void )
  { struct OPTION *option = g_try_malloc0( sizeof(struct OPTION) );
    if (!option)
     { Info_new( __func__, LOG_ERR, NULL, "memory error" ); }
    return(option);
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 GList *New_option_chaine( GList *options, gint token, gchar *chaine )
  { struct OPTION *option = g_try_malloc0( sizeof(struct OPTION) );
    if (!option)
     { Info_new( __func__, LOG_ERR, NULL, "Memory error for %s", chaine );
       return(options);
     }

    option->token        = token;
    option->token_classe = T_CHAINE;
    option->chaine       = g_strdup(chaine);
    return ( g_list_append ( options, option ) );
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 GList *New_option_entier( GList *options, gint token, gint entier )
  { struct OPTION *option;
    option=(struct OPTION *)g_try_malloc0( sizeof(struct OPTION) );
    if (!options) return(options);

    option->token        = token;
    option->token_classe = ENTIER;
    option->val_as_int   = entier;
    return ( g_list_append ( options, option ) );
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 gint Get_option_entier( GList *liste_options, gint token, gint defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token )
        { return (option->val_as_int); }
       liste = liste->next;
     }
    return(defaut);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 static struct ALIAS *Get_option_alias( GList *liste_options, gint token )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == ID )
        { return (option->val_as_alias); }
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Get_option_chaine: Cherche une option de type chaine et renvoie sa valeur                                                  */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 static gchar *Get_option_chaine( GList *liste_options, gint token, gchar *defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == T_CHAINE )
        { return (option->chaine); }
       liste = liste->next;
     }
    return(defaut);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée àla condition, en respectant les options                         */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_bi( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI_up ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI_down ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree: Prepare la chaine de caractere associée à la condition, en respectant les options                    */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_entree( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI_up ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI_down ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_sortie_tor: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_sortie_tor( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DO_up ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DO_down ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DO ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_sortie_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_sortie_ana( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (barre) Emettre_erreur_new ( scan_instance, "Use of / is forbidden in front of '%s'", alias->acronyme );
    else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AO(_%s_%s)", alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_mono: Prepare la chaine de caractere associée à la condition, en respectant les options                      */
/* Entrées: l'alias du monostable et sa liste d'options                                                                       */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_mono( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO_up ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO_down ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO ( _%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme );
     }
   return(condition);
 }
/******************************************************************************************************************************/
/* New_condition_tempo: Prepare la chaine de caractere associée à la condition, en respectant les options                     */
/* Entrées: l'alias de la temporisatio et sa liste d'options                                                                  */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_tempo( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_TEMPO ( _%s_%s )",
                (barre==1 ? "!" : ""), alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_horloge( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (!barre)
     { g_snprintf( condition->chaine, condition->taille, "Dls_data_get_HORLOGE ( _%s_%s )", alias->tech_id, alias->acronyme ); }
    else
     { g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_HORLOGE ( _%s_%s )", alias->tech_id, alias->acronyme ); }
   return(condition);
 }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_WATCHDOG( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (!barre)
     { g_snprintf( condition->chaine, condition->taille, "Dls_data_get_WATCHDOG ( _%s_%s )", alias->tech_id, alias->acronyme ); }
    else
     { g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_WATCHDOG ( _%s_%s )", alias->tech_id, alias->acronyme ); }
   return(condition);
 }
/******************************************************************************************************************************/
/* New_condition_comparateur: Prepare la chaine de caractere associée à la condition de comparateur                           */
/* Entrées: le tech_id/acronyme, ses options, son comparateur                                                                 */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_comparaison( void *scan_instance, struct CONDITION *condition_g, gint ordre, struct CONDITION *condition_d )
  { if (!condition_g) return(NULL);
    if (!condition_d) return(NULL);

    if (condition_g->is_bool == TRUE ) { Emettre_erreur_new ( scan_instance, "Boolean cannot be compared" ); return(NULL); }
    if (condition_d->is_bool == TRUE ) { Emettre_erreur_new ( scan_instance, "Boolean cannot be compared" ); return(NULL); }

    struct CONDITION *result = New_condition ( TRUE, condition_g->taille + condition_d->taille + 10 );
    if (!result) return(NULL);

    g_snprintf( result->chaine, result->taille, "%s", condition_g->chaine );

    switch(ordre)
     { case INF:         g_strlcat ( result->chaine, " < ", result->taille ); break;
       case SUP:         g_strlcat ( result->chaine, " > ", result->taille ); break;
       case INF_OU_EGAL: g_strlcat ( result->chaine, " <= ", result->taille ); break;
       case SUP_OU_EGAL: g_strlcat ( result->chaine, " >= ", result->taille ); break;
       case T_EGAL     : g_strlcat ( result->chaine, " == ", result->taille ); break;
     }
    g_strlcat ( result->chaine, condition_d->chaine, result->taille );
    return( result );
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_entree_ana( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  {
    gint in_range = Get_option_entier ( options, T_IN_RANGE, 0 );

    struct CONDITION *condition = New_condition( (in_range ? TRUE : FALSE), 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (in_range==1)
     { if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_AI_inrange(_%s_%s)",
                              alias->tech_id, alias->acronyme );
             else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AI_inrange(_%s_%s)",
                              alias->tech_id, alias->acronyme );
       return(condition);
     }
    if (barre) Emettre_erreur_new ( scan_instance, "Use of / is forbidden in front of '%s'", alias->acronyme );
    else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AI(_%s_%s)",
                     alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_registre( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) Emettre_erreur_new ( scan_instance, "Use of / is forbidden in front of '%s'", alias->acronyme );
    else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_REGISTRE(_%s_%s)",
                     alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_CI( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) Emettre_erreur_new ( scan_instance, "Use of / is forbidden in front of '%s'", alias->acronyme );
    else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_CI(_%s_%s)",
                     alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_CH( void *scan_instance, int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) Emettre_erreur_new ( scan_instance, "Use of / is forbidden in front of '%s'", alias->acronyme );
    else g_snprintf( condition->chaine, condition->taille, " Dls_data_get_CH(_%s_%s)", alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_comparateur: Prepare la chaine de caractere associée à la condition de comparateur                           */
/* Entrées: le tech_id/acronyme, ses options, son comparateur                                                                 */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_alias( void *scan_instance, gint barre, struct ALIAS *alias, GList *options )
  { if (!alias) return(NULL);

    switch(alias->classe)                                                  /* On traite que ce qui peut passer en "condition" */
     { case T_TEMPO :         return ( New_condition_tempo( scan_instance, barre, alias, options ) );
       case T_DIGITAL_INPUT:  return ( New_condition_entree( scan_instance, barre, alias, options ) );
       case T_DIGITAL_OUTPUT: return ( New_condition_sortie_tor( scan_instance, barre, alias, options ) );
       case T_BISTABLE:       return ( New_condition_bi( scan_instance, barre, alias, options ) );
       case T_MONOSTABLE:     return ( New_condition_mono( scan_instance, barre, alias, options ) );
       case T_HORLOGE:        return ( New_condition_horloge( scan_instance, barre, alias, options ) );
       case T_WATCHDOG:       return ( New_condition_WATCHDOG( scan_instance, barre, alias, options ) );
       case T_ANALOG_INPUT:   return ( New_condition_entree_ana( scan_instance, barre, alias, options ) );
       case T_ANALOG_OUTPUT:  return ( New_condition_sortie_ana( scan_instance, barre, alias, options ) );
       case T_REGISTRE:       return ( New_condition_registre( scan_instance, barre, alias, options ) );
       case T_CPT_IMP:        return ( New_condition_CI( scan_instance, barre, alias, options ) );
       case T_CPT_H:          return ( New_condition_CH( scan_instance, barre, alias, options ) );
       default:
        { Emettre_erreur_new ( scan_instance, "'%s' n'est pas une condition valide", alias->acronyme ); }
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* New_calcul_PID: Calcul un PID                                                                                              */
/* Entrées: la liste d'option associée au PID                                                                                 */
/* Sortie: la chaine de calcul DLS                                                                                            */
/******************************************************************************************************************************/
 gchar *New_calcul_PID ( void *scan_instance, GList *options )
  { struct ALIAS *input = Get_option_alias ( options, T_INPUT );
    if (!input)
     { Emettre_erreur_new ( scan_instance, "PID : input unknown. Select one R." );
       return(g_strdup("0"));
     }
    if ( input->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : input must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *consigne = Get_option_alias ( options, T_CONSIGNE );
    if (!consigne)
     { Emettre_erreur_new ( scan_instance, "PID : consigne unknown. Select one R." );
       return(g_strdup("0"));
     }
    if ( consigne->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : consigne must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kp = Get_option_alias ( options, T_KP );
    if (!kp)
     { Emettre_erreur_new ( scan_instance, "PID : kp. Select one R." );
       return(g_strdup("0"));
     }
    if ( kp->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kp must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *ki = Get_option_alias ( options, T_KD );
    if (!ki)
     { Emettre_erreur_new ( scan_instance, "PID : ki. Select one R." );
       return(g_strdup("0"));
     }
    if ( ki->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : ki must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kd = Get_option_alias ( options, T_KI );
    if (!kd)
     { Emettre_erreur_new ( scan_instance, "PID : kd. Select one R." );
       return(g_strdup("0"));
     }
    if ( kd->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kd must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_min = Get_option_alias ( options, T_MIN );
    if (!output_min)
     { Emettre_erreur_new ( scan_instance, "PID : output_min. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_min->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output_min must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_max = Get_option_alias ( options, T_MAX );
    if (!output_max)
     { Emettre_erreur_new ( scan_instance, "PID : output_max. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_max->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output_max must be R." );
       return(g_strdup("0"));
     }

    gint taille=512;
    gchar *chaine = New_chaine ( taille );
    g_snprintf( chaine, taille, "Dls_PID ( \"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s )",
                input->tech_id, input->acronyme, input->tech_id, input->acronyme,
                consigne->tech_id, consigne->acronyme, consigne->tech_id, consigne->acronyme,
                kp->tech_id, kp->acronyme, kp->tech_id, kp->acronyme,
                ki->tech_id, ki->acronyme, ki->tech_id, ki->acronyme,
                kd->tech_id, kd->acronyme, kd->tech_id, kd->acronyme,
                output_min->tech_id, output_min->acronyme, output_min->tech_id, output_min->acronyme,
                output_max->tech_id, output_max->acronyme, output_max->tech_id, output_max->acronyme );
    return(chaine);
  }
/******************************************************************************************************************************/
/* New_calcul_PID: Calcul un PID                                                                                              */
/* Entrées: la liste d'option associée au PID                                                                                 */
/* Sortie: la chaine de calcul DLS                                                                                            */
/******************************************************************************************************************************/
 struct ACTION *New_action_PID ( void *scan_instance, GList *options )
  { gint reset = Get_option_entier ( options, T_RESET, 0 );
    if (reset==0)
     { Emettre_erreur_new ( scan_instance, "PID : En action, l'option 'reset' est nécessaire." );
       return(NULL);
     }

    struct ALIAS *input = Get_option_alias ( options, T_INPUT );
    if (!input)
     { Emettre_erreur_new ( scan_instance, "PID : input unknown. Select one input (R or AI)." );
       return(NULL);
     }
    if ( ! (input->classe == T_REGISTRE /*|| input->classe == T_ANALOG_INPUT*/ ) )
     { Emettre_erreur_new ( scan_instance, "PID : input must be R or AI." );
       return(NULL);
     }

    struct ACTION *action = New_action();
    gint taille = 256;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "Dls_PID_reset ( \"%s\", \"%s\", &_%s_%s ); ",
                input->tech_id, input->acronyme, input->tech_id, input->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ACTION *New_action( void )
  { struct ACTION *action;
    action=(struct ACTION *)g_try_malloc0( sizeof(struct ACTION) );
    if (!action) { return(NULL); }
    action->alors = NULL;
    action->sinon = NULL;
    return(action);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition( gboolean is_bool, gint taille )
  { struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) { return(NULL); }
    condition->is_bool = is_bool;
    condition->taille = taille+1;
    if (taille)
     { condition->chaine = g_try_malloc0 ( taille );
       if (!condition->chaine) { g_free(condition); return(NULL); }
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct INSTRUCTION *New_instruction( void *scan_instance, struct CONDITION *condition, GList *options, struct ACTION *actions )
  { if (!condition) return(NULL);
    if (!actions)   return(NULL);
    if ( (condition->is_bool == TRUE  && actions->is_float == TRUE)
      || (condition->is_bool == FALSE && actions->is_float == FALSE)  )
     { Emettre_erreur_new( scan_instance, "Mix of bools and floats forbidden" );
       return(NULL);
     }
    struct INSTRUCTION *instr = g_try_malloc0( sizeof(struct INSTRUCTION) );
    if (!instr) return(NULL);
    instr->condition = condition;
    instr->options = options;
    instr->actions = actions;
    instr->line_number = DlsScanner_get_lineno(scan_instance);
    return (instr);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 void Del_instruction( struct INSTRUCTION *instr )
  { if (!instr) return;
    Del_condition (instr->condition);
    Del_actions (instr->actions);
    Liberer_options ( instr->options );
    g_free(instr);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 void Del_condition( struct CONDITION *condition )
  { if (!condition) return;
    if (condition->chaine) g_free(condition->chaine);
    g_free(condition);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 void Del_actions( struct ACTION *actions )
  { if (!actions) return;
    if (actions->alors) g_free(actions->alors);
    if (actions->sinon) g_free(actions->sinon);
    g_free(actions);
  }
/******************************************************************************************************************************/
/* New_condition_entier: Alloue une certaine quantité de mémoire pour les actions DLS                                         */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_entier( gint entier )
  { struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) { return(NULL); }
    condition->taille = 32;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "%d", entier );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_arcsin: Alloue une certaine quantité de mémoire pour la condition ARCSIN                                     */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_exp( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "exp(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_arcsin: Alloue une certaine quantité de mémoire pour la condition ARCSIN                                     */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_arcsin( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "asin(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_arctan: Alloue une certaine quantité de mémoire pour la condition ARCTAN                                     */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_arctan( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "atan(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_arccos: Alloue une certaine quantité de mémoire pour la condition ARCCOS                                     */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_arccos( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "acos(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_sin: Alloue une certaine quantité de mémoire pour la condition SIN                                           */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_sin( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "sin(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_tan: Alloue une certaine quantité de mémoire pour la condition TAN                                           */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_tan( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "tan(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_cos: Alloue une certaine quantité de mémoire pour la condition COS                                           */
/* Entrées: le parametre a passer a la fonction                                                                               */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_cos( struct CONDITION *parametre )
  { if (!parametre) return(NULL);
    if (parametre->is_bool == TRUE) return(NULL);
    struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 10 + parametre->taille;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "cos(%s)", parametre->chaine );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_valf: Alloue une certaine quantité de mémoire pour les actions DLS                                           */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct CONDITION *New_condition_valf( gdouble valf )
  { struct CONDITION *condition = g_try_malloc0( sizeof(struct CONDITION) );
    if (!condition) return(NULL);
    condition->taille = 32;
    condition->is_bool = FALSE;
    condition->chaine = g_try_malloc0 ( condition->taille );
    if (!condition->chaine) { g_free(condition); return(NULL); }
    g_snprintf ( condition->chaine, condition->taille, "%f", valf );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_action_msg: Prepare une struct action avec une commande de type MSG                                                    */
/* Entrées: L'alias decouvert                                                                                                 */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_msg( void *scan_instance, struct ALIAS *alias )
  { struct ACTION *action;
    gchar complement[256];

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ) )
     { Emettre_erreur_new ( scan_instance, "Setting MSG '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    if (alias->used!=1)
     { Emettre_erreur_new ( scan_instance, "Message %s could not be used more than once",  alias->acronyme );
       return(NULL);
     }

    action = New_action();
    gint taille_alors = 256;
    action->alors = g_try_malloc0 ( taille_alors );

    gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
    if (groupe)
     { GSList *liste = Dls_scanner->Alias;                        /* Parsing de tous les alias de type message du meme groupe */
       while (liste)
        { struct ALIAS *target_alias = liste->data;
          if (target_alias->classe == T_MSG && Get_option_entier ( target_alias->options, T_GROUPE, 0 ) == groupe &&
              target_alias != alias )
           { taille_alors += 256;
             action->alors = g_try_realloc ( action->alors, taille_alors );
             g_snprintf( complement, sizeof(complement), "   Dls_data_set_MESSAGE ( vars, _%s_%s, FALSE );\n",
                         target_alias->tech_id, target_alias->acronyme );
             g_strlcat ( action->alors, complement, taille_alors );
           }
          liste = g_slist_next(liste);
        }
     }
    gint debug = Get_option_entier ( alias->options, T_DEBUG,  0 );
    if (debug)
     { g_snprintf( complement, sizeof(complement), "   if (vars->debug) Dls_data_set_MESSAGE ( vars, _%s_%s, TRUE );\n",  alias->tech_id, alias->acronyme ); }
    else
     { g_snprintf( complement, sizeof(complement), "   Dls_data_set_MESSAGE ( vars, _%s_%s, TRUE );\n",  alias->tech_id, alias->acronyme ); }
    g_strlcat ( action->alors, complement, taille_alors );

    if(!groupe)
     { gint taille_sinon = 256;
       action->sinon = New_chaine( taille_sinon );
       g_snprintf( action->sinon, taille_sinon, "   Dls_data_set_MESSAGE ( vars, _%s_%s, FALSE );\n", alias->tech_id, alias->acronyme );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_sortie: Prepare la structure ACTION associée à l'alias en paramètre                                             */
/* Entrées: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associée                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_sortie( void *scan_instance, struct ALIAS *alias, int barre )
  { struct ACTION *action = New_action();

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ) )
     { Emettre_erreur_new ( scan_instance, "Setting DO '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gint taille = 128;
    action->alors = New_chaine( taille );
    if ( (!barre) )
         { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, _%s_%s, TRUE );\n", alias->tech_id, alias->acronyme ); }
    else { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, _%s_%s, FALSE );\n", alias->tech_id, alias->acronyme ); }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_mono( void *scan_instance, struct ALIAS *alias )
  { struct ACTION *action;
    gchar complement[256];

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ) )
     { Emettre_erreur_new ( scan_instance, "Setting MONO '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    action = New_action();
    gint taille_alors = 256;
    action->alors = g_try_malloc0 ( taille_alors );

    gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
    if (groupe)
     { GSList *liste = Dls_scanner->Alias;                           /* Parsing de tous les alias de type message du meme groupe */
       while (liste)
        { struct ALIAS *target_alias = liste->data;
          if (target_alias->classe == T_MONOSTABLE && Get_option_entier ( target_alias->options, T_GROUPE, 0 ) == groupe &&
              target_alias != alias )
           { taille_alors += 256;
             action->alors = g_try_realloc ( action->alors, taille_alors );
             g_snprintf( complement, sizeof(complement), "   Dls_data_set_MONO ( vars, _%s_%s, FALSE );\n",
                         target_alias->tech_id, target_alias->acronyme );
             g_strlcat ( action->alors, complement, taille_alors );
           }
          liste = g_slist_next(liste);
        }
     }
    g_snprintf( complement, sizeof(complement), "   Dls_data_set_MONO ( vars, _%s_%s, TRUE );\n", alias->tech_id, alias->acronyme );
    g_strlcat ( action->alors, complement, taille_alors );

    gint taille_sinon = 256;
    action->sinon = New_chaine( taille_sinon );
    g_snprintf( action->sinon, taille_sinon, "   Dls_data_set_MONO ( vars, _%s_%s, FALSE);\n", alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_DI: Prepare une struct action avec une commande DigitalInput                                                    */
/* Entrées: l'alias de la digital input                                                                                       */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_DI( void *scan_instance, struct ALIAS *alias )
  { struct ACTION *action;

    action = New_action();
    gint taille_alors = 256;
    action->alors = g_try_malloc0 ( taille_alors );

    g_snprintf( action->alors, taille_alors, "   if(prev_state!=1) Dls_data_set_DI_pulse ( vars, _%s_%s );\n", alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_cpt_h: Prepare une struct action avec une commande CPTH                                                         */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_h( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action;

    gint reset = Get_option_entier ( all_options, T_RESET, 0 );

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ) && reset==0)
     { Emettre_erreur_new ( scan_instance, "Setting CH '%s:%s' without 'reset' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CH ( vars, _%s_%s, TRUE,  %d );\n", alias->tech_id, alias->acronyme, reset );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CH ( vars, _%s_%s, FALSE, %d );\n", alias->tech_id, alias->acronyme, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_cpt_imp: Prepare une struct action avec une commande CPT_IMP                                                    */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_imp( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action;

    gint reset = Get_option_entier ( all_options, T_RESET, 0 );

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ) && reset==0)
     { Emettre_erreur_new ( scan_instance, "Setting CI '%s:%s' without 'reset' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CI ( vars, _%s_%s, TRUE, %d );\n",
                alias->tech_id, alias->acronyme, reset );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CI ( vars, _%s_%s, FALSE, %d );\n",
                alias->tech_id, alias->acronyme, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_WATCHDOG: Prepare une struct action pour une action de type WATCHDOG                                            */
/* Entrées: l'alias source, et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_WATCHDOG( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action;

    struct ALIAS *alias_consigne = Get_option_alias ( all_options, T_CONSIGNE );
    if (alias_consigne)
     { if (alias_consigne->classe != T_REGISTRE)
        { Emettre_erreur_new ( scan_instance, "'%s:%s' is not a REGISTER. Setting in Watchdog '%s' is not allowed.",
                               alias_consigne->tech_id, alias_consigne->acronyme, alias->acronyme );
          return(NULL);
        }

       gint taille = 512;
       action = New_action();
       action->alors = New_chaine( taille );

       g_snprintf( action->alors, taille,
                   "   if (prev_state!=1) Dls_data_set_WATCHDOG ( vars, _%s_%s, Dls_data_get_REGISTRE ( _%s_%s ) );\n",
                   alias->tech_id, alias->acronyme,
                   alias_consigne->tech_id, alias_consigne->acronyme
                 );
       return(action);
     }

    gint consigne = Get_option_entier ( all_options, T_CONSIGNE, 600 );
    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   if (prev_state!=1) Dls_data_set_WATCHDOG ( vars, _%s_%s, %d );\n",
                alias->tech_id, alias->acronyme, consigne );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_registre: Prepare une struct action avec une commande registre                                                  */
/* Entrées: l'alias associé et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_REGISTRE( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action;

    gint taille = 256;
    action = New_action();
    action->is_float = TRUE;
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_REGISTRE ( vars, _%s_%s, local_result );\n",
                alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_AO: Prepare une struct action avec une commande analog Output                                                   */
/* Entrées: l'alias associé et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_AO( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action;
    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ))
     { Emettre_erreur_new ( scan_instance, "Setting AO '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gint taille = 256;
    action = New_action();
    action->is_float = TRUE;
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_AO ( vars, _%s_%s, local_result );\n",
                alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_visuel: Prepare une struct action avec une commande SI                                                           */
/* Entrées: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_visuel( void *scan_instance, struct ALIAS *alias, GList *all_options )
  { struct ACTION *action = NULL;
    int taille;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ))
     { Emettre_erreur_new ( scan_instance, "Setting VISUEL '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gchar *forme        = Get_option_chaine ( all_options, T_FORME, NULL );
    gchar *mode         = Get_option_chaine ( all_options, T_MODE,  "default_mode" );
    gchar *couleur      = Get_option_chaine ( all_options, T_COLOR, "black" );
    gint   cligno       = Get_option_entier ( all_options, CLIGNO, 0 );
    gint   noshow       = Get_option_entier ( all_options, T_NOSHOW, 0 );
    gint   disable      = Get_option_entier ( all_options, T_DISABLE, 0 );
    gchar *libelle      = Get_option_chaine ( all_options, T_LIBELLE, "pas de libellé" );
    struct ALIAS *input = Get_option_alias  ( all_options, T_INPUT );

    if ( ! Dls_check_mode_VISUEL ( Dls_scanner, forme, mode ) )
     { Emettre_erreur_new ( scan_instance, "'%s:%s': mode '%s' is not known for forme '%s'", alias->tech_id, alias->acronyme, mode, forme );
       return(NULL);
     }

    if (!input)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, _%s_%s, \"%s\", \"%s\", 0.0, %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, mode, couleur, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_ANALOG_INPUT)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, _%s_%s, \"%s\", \"%s\", Dls_data_get_AI (_%s_%s), %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, mode, couleur, input->tech_id, input->acronyme, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_CPT_IMP)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL_for_CI( vars, _%s_%s, _%s_%s, \"%s\", \"%s\", %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, mode, couleur, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_CPT_H)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, _%s_%s, \"%s\", \"%s\", Dls_data_get_CH (_%s_%s), %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, mode, couleur, input->tech_id, input->acronyme, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_REGISTRE)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL_for_REGISTRE( vars, _%s_%s, _%s_%s, \"%s\", \"%s\", %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, mode, couleur, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_WATCHDOG)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       mode="horaire";                                 /* Par défaut toutes les watchdog sont affichées en mode cadran horaire */
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL_for_WATCHDOG( vars, _%s_%s, _%s_%s, \"%s\", \"%s\", %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, mode, couleur, cligno, noshow, libelle, disable );
     }
    else if (input->classe == T_TEMPO)
     { action = New_action();
       taille = 768;
       action->alors = New_chaine( taille );
       mode="horaire";                                 /* Par défaut toutes les watchdog sont affichées en mode cadran horaire */
       g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL_for_TEMPO( vars, _%s_%s, _%s_%s, \"%s\", \"%s\", %d, %d, \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, mode, couleur, cligno, noshow, libelle, disable );
     }
    else Emettre_erreur_new ( scan_instance, "'%s:%s' is not allowed in 'input'", input->tech_id, input->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                                                   */
/* Entrées: un commentaire (gchar *)                                                                                          */
/* Sortie: boolean false si probleme                                                                                          */
/******************************************************************************************************************************/
 static gchar *Normaliser_chaine_for_dls( gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                                           /* Validate la chaine */
    comment = g_try_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );                  /* Au pire, ts les car sont doublés */
                                                                                                      /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_new( __func__, LOG_WARNING, NULL, "Memory error %s", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;

    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\"' )                                                                   /* Remplacement de la double cote */
        { g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\"", 1 ); cible = g_utf8_next_char( cible );
        }
       else if ( car == '\n' )                                                              /* Remplacement de la double cote */
        { /* Supprime les \n */ }
       else
        { g_utf8_strncpy( cible, source, 1 ); cible = g_utf8_next_char( cible );
        }
       source = g_utf8_next_char(source);
     }
    return(comment);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bus( void *scan_instance,struct ALIAS *alias, GList *all_options )
  { struct ACTION *result;
    gchar *option_chaine;
    gint taille;

    JsonNode *RootNode = Json_node_create ();

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *target_tech_id = Get_option_chaine ( all_options, T_TECH_ID, Json_get_string ( Dls_scanner->PluginNode, "tech_id" ) );
    Json_node_add_string ( RootNode, "thread_tech_id", target_tech_id );

    option_chaine = Get_option_chaine ( all_options, T_TAG, "PING" );
    if (option_chaine) Json_node_add_string ( RootNode, "tag", option_chaine );

    option_chaine = Get_option_chaine ( all_options, T_COMMAND, NULL );
    if (option_chaine) Json_node_add_string ( RootNode, "command", option_chaine );

    gchar *json_buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
    gchar *normalized_buf = Normaliser_chaine_for_dls ( json_buf );
    g_free(json_buf);

    result = New_action();
    taille = 256+strlen(target_tech_id)+strlen(json_buf);
    result->alors = New_chaine( taille );
    g_snprintf( result->alors, taille, "  Dls_data_set_bus ( vars, \"%s\", TRUE );\n", normalized_buf );
    g_free(normalized_buf);
    return(result);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_tempo( void *scan_instance, struct ALIAS *alias )
  { struct ACTION *action;
    int taille, daa, dma, dMa, dad, random;

    daa    = Get_option_entier ( alias->options, T_DAA, 0 );
    dma    = Get_option_entier ( alias->options, T_DMINA, 0 );
    dMa    = Get_option_entier ( alias->options, T_DMAXA, 0 );
    dad    = Get_option_entier ( alias->options, T_DAD, 0 );
    random = Get_option_entier ( alias->options, T_RANDOM, 0 );

    action = New_action();
    taille = 256;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille,
                "   Dls_data_set_TEMPO ( vars, _%s_%s, 1, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille,
                "   Dls_data_set_TEMPO ( vars, _%s_%s, 0, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_bi: Prepare une struct action avec une commande BI                                                              */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bi( void *scan_instance, struct ALIAS *alias, gint barre )
  { struct ACTION *action;
    gchar complement[256];

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
    if (groupe && barre)
     { Emettre_erreur_new ( scan_instance, "Bistable '%s:%s' could not be in a group and used with '/'",  alias->tech_id, alias->acronyme );
       return(NULL);
     }

    action = New_action();
    gint taille_alors = 256;
    action->alors = g_try_malloc0 ( taille_alors );

    if (groupe)
     { GSList *liste = Dls_scanner->Alias;                           /* Parsing de tous les alias de type message du meme groupe */
       while (liste)
        { struct ALIAS *target_alias = liste->data;
          if (target_alias->classe == T_BISTABLE && Get_option_entier ( target_alias->options, T_GROUPE, 0 ) == groupe &&
              target_alias != alias )
           { taille_alors += 256;
             action->alors = g_try_realloc ( action->alors, taille_alors );
             g_snprintf( complement, sizeof(complement), "   Dls_data_set_BI ( vars, _%s_%s, FALSE );\n",
                         target_alias->tech_id, target_alias->acronyme );
             g_strlcat ( action->alors, complement, taille_alors );
           }
          liste = g_slist_next(liste);
        }
     }

    taille_alors += 256;
    action->alors = g_try_realloc ( action->alors, taille_alors );
    g_snprintf( complement, sizeof(complement),
                "   Dls_data_set_BI ( vars, _%s_%s, %s );\n",
                alias->tech_id, alias->acronyme, (barre ? "FALSE" : "TRUE") );
    g_strlcat ( action->alors, complement, taille_alors );

    return(action);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 static gdouble Get_option_double( GList *liste_options, gint token, gdouble defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == T_VALF )
        { return (option->val_as_double); }
       liste = liste->next;
     }
    return(defaut);
  }
/******************************************************************************************************************************/
/* New_alias_systeme: Créé un alias pour le système                                                                           */
/* Entrées: le tech_id/acronyme de l'alias, sa classe et ses options                                                          */
/* Sortie: La structure Alias associée                                                                                        */
/******************************************************************************************************************************/
 static struct ALIAS *New_alias_systeme ( void *scan_instance, gchar *acronyme, gint classe, GList *options )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    struct ALIAS *alias = Get_local_alias ( scan_instance, plugin_tech_id, acronyme );                  /* Si pas déjà défini */
    if (!alias)
     { alias = New_alias ( scan_instance, plugin_tech_id, acronyme, classe, options );
       if (alias)
        { alias->used=1;                                                              /* Un alias permanent est toujours used */
          alias->systeme = TRUE;
        }
     }
    return(alias);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le tech_id/Acronyme de l'alias                                                                                    */
/* Sortie: la structure, ou FALSE si erreur                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *New_alias ( void *scan_instance, gchar *tech_id, gchar *acronyme, gint classe, GList *options )
  { struct ALIAS *alias;
    gchar chaine[256];


    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );

    if (!acronyme)
     { Emettre_erreur_new ( Dls_scanner->scan_instance, "Acronyme is null" );
       return(NULL);
     }

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(NULL); }
    if (!tech_id) alias->tech_id = g_strdup( plugin_tech_id );
             else alias->tech_id = g_strdup( tech_id );
    alias->acronyme = g_strdup(acronyme);
    alias->classe   = classe;
    alias->options  = options;
    alias->used     = 0;
    Dls_scanner->Alias = g_slist_prepend( Dls_scanner->Alias, alias );
    Info_new( __func__, LOG_DEBUG, NULL, "'%s:%s'", alias->tech_id, alias->acronyme );

    gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, "no libelle" );
    switch(alias->classe)
     { case T_BUS:
        { g_snprintf(chaine, sizeof(chaine), " static gpointer _%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_BISTABLE:
        { gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
          if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_BI ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle, groupe );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_BI *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_MONOSTABLE:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_MONO ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_MONO *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_DIGITAL_INPUT:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_DI_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_DI *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_DIGITAL_OUTPUT:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_DO_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_DO *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_ANALOG_OUTPUT:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_AO_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_AO *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_ANALOG_INPUT:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_AI_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_AI *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_TEMPO:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_TEMPO ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_TEMPO *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_HORLOGE:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_HORLOGE_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_HORLOGE *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_REGISTRE:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_REGISTRE ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle,
                                                                                    Get_option_chaine( alias->options, T_UNITE, "unit" ) );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_REGISTRE *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_WATCHDOG:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_WATCHDOG_from_dls ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_WATCHDOG *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
/*--------------------------------------------- New Alias Visuel : Controle des paramètres +++++++++++++++++++++++++++++++++++*/
       case T_VISUEL:
        { if ( strcmp(alias->tech_id, plugin_tech_id) )               /* Usage d'un visuel d'un autre DLS ?? -> c'est un link */
           { Emettre_erreur_new ( scan_instance, "'%s:%s': could not use foreign VISUEL", alias->tech_id, alias->acronyme );
             break;
           }

          gchar *forme        = Get_option_chaine ( alias->options, T_FORME, NULL );
          struct ALIAS *input = Get_option_alias  ( alias->options, T_INPUT );

          if ( input && !forme )                              /* Si un input sur forme, par défaut on prend la forme 'cadran' */
           { forme="cadran";
             alias->options = New_option_chaine ( alias->options, T_FORME, forme );
           }
          if (!input && !forme )                                      /* Si pas d'input, pas de forme, par défaut -> question */
           { forme="question";
             alias->options = New_option_chaine ( alias->options, T_FORME, forme );
           }

          gchar *forme_safe = Normaliser_chaine ( forme );
          if (!forme_safe) { Emettre_erreur_new ( scan_instance, "'%s:%s': memory error", alias->tech_id, alias->acronyme ); break; }

          JsonNode *RootNode = Json_node_create();
          if ( RootNode &&                                  /* Chargement des parametres en base de données pour vérification */
               DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL,
                         "SELECT icon_id, default_mode, default_color FROM icons WHERE forme='%s'", forme_safe ) &&
               Json_has_member ( RootNode, "icon_id" ) )
           { gchar *couleur = Get_option_chaine( alias->options, T_COLOR, NULL );
             if (!couleur)
              { couleur = Json_get_string ( RootNode, "default_color" );
                alias->options = New_option_chaine ( alias->options, T_COLOR, couleur );
              }
             gchar *mode    = Get_option_chaine( alias->options, T_MODE, NULL );
             if (!mode)
              { mode = Json_get_string ( RootNode, "default_mode" );
                alias->options = New_option_chaine ( alias->options, T_MODE, mode );
              }

             if ( Dls_check_mode_VISUEL ( Dls_scanner, forme, mode ) )
              { gdouble min        = Get_option_double ( alias->options, T_MIN,       0   );
                gdouble max        = Get_option_double ( alias->options, T_MAX,       100 );
                gdouble seuil_ntb  = Get_option_double ( alias->options, T_SEUIL_NTB, 10  );
                gdouble seuil_nb   = Get_option_double ( alias->options, T_SEUIL_NB,  20  );
                gdouble seuil_nh   = Get_option_double ( alias->options, T_SEUIL_NH,  80  );
                gdouble seuil_nth  = Get_option_double ( alias->options, T_SEUIL_NTH, 100 );
                gint    nb_decimal = Get_option_entier ( alias->options, T_DECIMAL,   2   );
                Mnemo_auto_create_VISUEL ( Dls_scanner->domain, Dls_scanner->PluginNode, alias->acronyme, libelle, forme, mode, couleur,
                                           min, max, seuil_ntb, seuil_nb, seuil_nh, seuil_nth, nb_decimal,
                                           (input ? input->tech_id : ""), (input ? input->acronyme : "")
                                         );
                Synoptique_auto_create_MOTIF ( Dls_scanner->domain, Dls_scanner->PluginNode, alias->tech_id, alias->acronyme, Dls_scanner->visuel_place++ );
              }
             else
              { Emettre_erreur_new ( scan_instance, "'%s:%s': mode '%s' is not known for forme '%s'", alias->tech_id, alias->acronyme, mode, forme ); }
           }
          else
           { Emettre_erreur_new ( scan_instance, "'%s:%s': forme '%s' is not known", alias->tech_id, alias->acronyme, forme ); }

          if (RootNode) json_node_unref ( RootNode );
          g_free(forme_safe);

          gchar ss_acronyme[64];
          g_snprintf( ss_acronyme, sizeof(ss_acronyme), "%s_CLIC", acronyme );
          GList *options = New_option_chaine ( NULL, T_LIBELLE, "Clic sur le visuel depuis l'IHM" );
          New_alias_systeme ( scan_instance, ss_acronyme, T_DIGITAL_INPUT, options );

          g_snprintf(chaine, sizeof(chaine), " static struct DLS_VISUEL *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_CPT_IMP:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_CI ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle,
                                                                              Get_option_chaine ( alias->options, T_UNITE, "fois" ) );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_CI *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_CPT_H:
        { if (!strcmp(alias->tech_id, plugin_tech_id)) Mnemo_auto_create_CH ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_CH *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
       case T_MSG:
        { if ( strcmp(alias->tech_id, plugin_tech_id) )                 /* Usage d'un message d'un autre DLS ?? -> forbiddent */
           { Emettre_erreur_new ( scan_instance, "'%s:%s': could not use foreign MSG", alias->tech_id, alias->acronyme );
             break;
           }
          gint type      = Get_option_entier ( alias->options, T_TYPE, MSG_ETAT );
          gint notif_sms = Get_option_entier ( alias->options, T_NOTIF_SMS, T_NO );
          switch (notif_sms)
           { case T_NO:        notif_sms = 0; break;
             case T_YES:       notif_sms = 1; break;
             case T_OVH_ONLY : notif_sms = 2; break;
           }
          gint notif_chat = Get_option_entier ( alias->options, T_NOTIF_CHAT, T_YES );
          switch (notif_chat)
            {case T_NO:        notif_chat = 0; break;
             case T_YES:       notif_chat = 1; break;
           }
          if (!strcmp(alias->tech_id, plugin_tech_id))
           { Mnemo_auto_create_MSG ( Dls_scanner->domain, TRUE, alias->tech_id, alias->acronyme, libelle, type, notif_sms, notif_chat ); }
          g_snprintf(chaine, sizeof(chaine), " static struct DLS_MESSAGE *_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
          Emettre( Dls_scanner->scan_instance, chaine );
          break;
        }
     }
    return(alias);
  }
/******************************************************************************************************************************/
/* New_link: Alloue une certaine quantité de mémoire pour utiliser des link                                                   */
/* Entrées: le tech_id et l'acronyme du link, et les options eventuelles                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void New_link( void *scan_instance, gchar *tech_id, gchar *acronyme, GList *options )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (!strcmp ( plugin_tech_id, tech_id ) )                                                     /* Si tech_id local, erreur */
     { Emettre_erreur_new ( scan_instance, "'%s:%s': tech_id forbidden", tech_id, acronyme );
       return;
     }

    JsonNode *result = Rechercher_DICO ( Dls_scanner->domain, tech_id, acronyme );
    if (!result)
     { Info_new( __func__, LOG_ERR, Dls_scanner->domain, "'%s:%s' not found in DICO", tech_id, acronyme );
       Emettre_erreur_new ( scan_instance, "'%s:%s': not found in DICO", tech_id, acronyme );
       return;
     }

    if ( ! Json_has_member ( result, "classe" ) )
     { Emettre_erreur_new ( scan_instance, "'%s:%s': classe not found in DICO", tech_id, acronyme );
       goto end;
     }

    if (strcmp ( Json_get_string ( result, "classe" ), "VISUEL" ) )
     { Emettre_erreur_new ( scan_instance, "'%s:%s': is not a VISUEL", tech_id, acronyme );
       goto end;
     }

     Synoptique_auto_create_MOTIF ( Dls_scanner->domain, Dls_scanner->PluginNode, tech_id, acronyme, Dls_scanner->visuel_place++ );
end:
    json_node_unref ( result );
  }
/******************************************************************************************************************************/
/* New_parametre: Créé un parametre pour le DLS en cours de compilation                                                       */
/* Entrées: l'acronyme du parametre, et les options eventuelles                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void New_parametre( void *scan_instance, gchar *acronyme, GList *options )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );

    gchar *libelle_safe = Normaliser_chaine ( Get_option_chaine( options, T_LIBELLE, "default libelle" ) );/* Formatage correct des chaines */
    gchar *defaut       = Get_option_chaine( options, T_DEFAUT, NULL );
    if (!defaut)
     { gint defaut_int = Get_option_entier( options, T_DEFAUT, 0 );
       DB_Write ( Dls_scanner->domain, "INSERT INTO dls_params SET tech_id='%s', acronyme='%s', libelle='%s', valeur='%d' "
                                       "ON DUPLICATE KEY UPDATE libelle = VALUE(libelle)",
                                        plugin_tech_id, acronyme, libelle_safe, defaut_int );
     }
    else
     { gchar *defaut_safe = Normaliser_chaine ( defaut );                                    /* Formatage correct des chaines */
       DB_Write ( Dls_scanner->domain, "INSERT INTO dls_params SET tech_id='%s', acronyme='%s', libelle='%s', valeur='%s' "
                                       "ON DUPLICATE KEY UPDATE libelle = VALUE(libelle)",
                                        plugin_tech_id, acronyme, libelle_safe, defaut_safe );
       g_free(defaut_safe);
     }
    g_free(libelle_safe);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 struct ALIAS *New_external_alias( void *scan_instance, gchar *tech_id, gchar *acronyme, GList *options )
  { struct ALIAS *alias=NULL;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (!tech_id) tech_id=plugin_tech_id;         /* Cas d'usage : bit créé par un thread, n'ayant pas été defini dans le DLS */

    JsonNode *result = Rechercher_DICO ( Dls_scanner->domain, tech_id, acronyme );
    if (!result)
     { Info_new( __func__, LOG_ERR, Dls_scanner->domain, "'%s:%s'. Error when searching in DICO", tech_id, acronyme );
       return(NULL);
     }

    if ( !Json_has_member ( result, "classe" ) ) goto end;                                                   /* Si pas trouvé */
               /* Si c'est un bit local du plugin en cours, mais qu"il est deletable, c'est que sa définition a été supprimée */
    if (!strcmp ( tech_id, plugin_tech_id ) && Json_get_bool ( result, "deletable" ) ) goto end;

    if (!strcmp ( Json_get_string ( result, "classe" ), "VISUEL" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_VISUEL, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "DI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_DIGITAL_INPUT, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "DO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_DIGITAL_OUTPUT, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "AI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_ANALOG_INPUT, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "AO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_ANALOG_OUTPUT, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "HORLOGE" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_HORLOGE, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "WATCHDOG" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_WATCHDOG, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "MONO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_MONOSTABLE, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "BI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_BISTABLE, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "CI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_CPT_IMP, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "CH" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_CPT_H, options ); }
    else if ( !strcmp ( Json_get_string ( result, "classe" ), "REGISTRE" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, T_REGISTRE, options ); }

end:
    json_node_unref ( result );

    if (alias)                                                                 /* Si trouvé, on considère que le bit est used */
     { alias->used = 1;
       Info_new( __func__, LOG_DEBUG, Dls_scanner->domain, "'%s:%s' found", alias->tech_id, alias->acronyme );
     }
    else { Info_new( __func__, LOG_ERR, Dls_scanner->domain, "'%s:%s' new_alias not found", tech_id, acronyme ); }
    return(alias);
  }
/******************************************************************************************************************************/
/* Get_local_alias: Recherche un alias donné en paramètre                                                                     */
/* Entrées: le nom de l'alias                                                                                                 */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *Get_local_alias( void *scan_instance, gchar *tech_id, gchar *acronyme )
  { struct ALIAS *alias;
    GSList *liste;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    liste = Dls_scanner->Alias;

    if (tech_id == NULL) tech_id = plugin_tech_id;

    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if (alias->acronyme && alias->tech_id &&
              !strcmp(alias->acronyme, acronyme) &&
            ( !strcmp(alias->tech_id, tech_id) || !strcmp(alias->tech_id,"SYS") )
          )
        { alias->used++; return(alias); }                                          /* Si deja present, on fait ++ sur le used */
       liste = liste->next;
     }
    return (NULL);
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de mémoire précédemment allouées                                             */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Liberer_options ( GList *options )
  { while (options)
     { struct OPTION *option = (struct OPTION *)options->data;
       options = g_list_remove (options, option);
       if (option->token_classe == T_CHAINE) g_free(option->chaine);
       g_free(option);
     }
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de mémoire précédemment allouées                                             */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_alias ( struct ALIAS *alias )
  { Liberer_options(alias->options);
    if (alias->tech_id)  g_free(alias->tech_id);
    if (alias->acronyme) g_free(alias->acronyme);
    g_free(alias);
  }
/******************************************************************************************************************************/
/* End_scanner: Ferme le scanner en parametre                                                                                 */
/* Entrée: le scanner                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void End_scanner ( struct DLS_TRAD *scanner )
  { if (!scanner) return;

    if (scanner->scan_instance) DlsScanner_lex_destroy (scanner->scan_instance);
    if (scanner->Alias)
     { g_slist_foreach( scanner->Alias, (GFunc) Liberer_alias, NULL );
       g_slist_free( scanner->Alias );
       scanner->Alias = NULL;
     }
    if (scanner->Visuel_check_cache)
     { g_slist_free_full( scanner->Visuel_check_cache, (GDestroyNotify) json_node_unref );
       scanner->Visuel_check_cache = NULL;
     }
    if (scanner->Buffer) { g_free(scanner->Buffer); scanner->Buffer = NULL; }
    if (scanner->Error)  { g_free(scanner->Error);  scanner->Error  = NULL; }
    g_free(scanner);
  }
/******************************************************************************************************************************/
/* New_scanner: Prepare un nouveau scanner pour traduire le D.L.S                                                             */
/* Entrée: le domain d'application, le node du module dls                                                                     */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 static struct DLS_TRAD *New_scanner ( struct DOMAIN *domain, JsonNode *PluginNode )
  { gchar *tech_id = Json_get_string ( PluginNode, "tech_id" );

    Json_node_add_bool ( PluginNode, "compil_status", FALSE );
    Json_node_add_int  ( PluginNode, "compil_time", 0 );
    struct DLS_TRAD *scanner = g_try_malloc0 ( sizeof ( struct DLS_TRAD ) );
    if (!scanner)
     { Info_new( __func__, LOG_ERR, domain, "'%s': DLS_TRAD memory error", tech_id );
       Json_node_add_string ( PluginNode, "errorlog", "Memory Scanner Error" );
       return(NULL);
     }
    scanner->PluginNode   = PluginNode;
    scanner->domain       = domain;
    scanner->nbr_erreur   = 0;                                                        /* Au départ, nous n'avons pas d'erreur */
    scanner->visuel_place = 0;                                                       /* Au départ, nous n'avons pas de visuel */

    scanner->buffer_size = 1024;
    scanner->Buffer = g_try_malloc0( scanner->buffer_size+1 );                           /* Initialisation du buffer resultat */
    if (!scanner->Buffer)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Not enought memory for buffer", tech_id );
       Json_node_add_string ( PluginNode, "errorlog", "Memory error for buffer" );
       End_scanner ( scanner );
       return(NULL);
     }
    scanner->buffer_used = 0;

    scanner->Error = g_try_malloc0( 1 );                                                 /* Initialisation du buffer resultat */
    if (!scanner->Error)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Not enought memory for ErrorBuffer", tech_id );
       Json_node_add_string ( PluginNode, "compil_error", "Memory error for ErrorBuffer" );
       End_scanner ( scanner );
       return(NULL);
     }

    DlsScanner_lex_init (&scanner->scan_instance);
    DlsScanner_debug = Json_get_bool ( domain->config, "debug_dls" );
    DlsScanner_set_extra( (void *)scanner, scanner->scan_instance );

    return(scanner);
  }
/******************************************************************************************************************************/
/* Add_unused_as_action_visuels: Ajoute le pilotage forcés des visuels qui ne sont pas settés par le DLS.                     */
/* Entrée: le scanner en cours                                                                                                */
/* Sortie: résultat dans le scanner lui meme                                                                                  */
/******************************************************************************************************************************/
 void Add_unused_as_action_visuels ( void *scan_instance )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    Emettre ( scan_instance, "\n /************ Add unused_as_action_visuels, if any */\n" );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    GSList *liste = Dls_scanner->Alias;                                           /* Set_Visuel pour tous les alias du module */
    while(liste)
     { struct ALIAS *alias = liste->data;
       if ( alias->used_as_action == FALSE && alias->classe == T_VISUEL && !strcasecmp ( alias->tech_id, plugin_tech_id ))
        { alias->used++;
          struct ACTION *action = New_action_visuel ( scan_instance, alias, alias->options );
          if (action)
           { Emettre ( scan_instance, action->alors );
             Del_actions ( action );
           }
        }
       liste = g_slist_next ( liste );
     }
    Emettre ( scan_instance, "/************ End of Add unused_as_action_visuels ***************/" );
  }
/******************************************************************************************************************************/
/* Dls_traduire_plugin: Traduction du fichier en paramètre du langage DLS vers le langage C                                   */
/* Entrée: le domaine d'application et le PluginNode                                                                          */
/* Sortie: résultat dans le PluginNode                                                                                        */
/******************************************************************************************************************************/
 void Dls_traduire_plugin ( struct DOMAIN *domain, JsonNode *PluginNode )
  { struct ALIAS *alias;
    gchar source[256];
    GSList *liste;

    if (!PluginNode)
     { Info_new( __func__, LOG_ERR, domain, "DLS_TRAD PluginNode Error (is null)" );
       return;
     }
    gint compil_top    = Global.Top;
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gchar *tech_id     = Json_get_string ( PluginNode, "tech_id" );
    Json_node_add_int ( PluginNode, "error_count",   0 );
    Json_node_add_int ( PluginNode, "warning_count", 0 );

    Info_new( __func__, LOG_INFO, domain, "'%s': Starting traduction.", tech_id );
    DB_Write ( domain, "UPDATE dls SET nbr_compil=nbr_compil+1 WHERE tech_id='%s'", tech_id );
/************************************************ Descend le sourcecode sur disque ********************************************/
    g_snprintf( source, sizeof(source), "/tmp/%s-%s.dls", domain_uuid, tech_id );
    unlink ( source );
    gint fd_source = open( source, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd_source<0)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Source creation failed %s (%s)", tech_id, source, strerror(errno) );
       Json_node_add_string ( PluginNode, "compil_error", "Source creation failed" );
       return;
     }
    gchar *sourcecode_to_write = Json_get_string ( PluginNode, "sourcecode" );
    if (sourcecode_to_write) write ( fd_source, sourcecode_to_write, strlen (sourcecode_to_write) );
    close(fd_source);

/*********************************** Prépare la détection des bits unused en base de données **********************************/
    Info_new( __func__, LOG_INFO, domain, "'%s': Preparing DB to detect unused bits", tech_id );
    DB_Write ( domain, "UPDATE `syns_motifs`     SET used=0 WHERE dls_id=%d", Json_get_int ( PluginNode, "dls_id" ) );
    DB_Write ( domain, "UPDATE `mnemos_VISUEL`   SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_BI`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_MONO`     SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_CI`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_CH`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_DI`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_DO`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_AI`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_AO`       SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_HORLOGE`  SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_REGISTRE` SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_WATCHDOG` SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `mnemos_TEMPO`    SET used=0 WHERE tech_id='%s'", tech_id );
    DB_Write ( domain, "UPDATE `msgs`            SET used=0 WHERE tech_id='%s'", tech_id );

/************************************************ Charge un scanner ***********************************************************/
    struct DLS_TRAD *Dls_scanner = New_scanner ( domain, PluginNode );
    if (!Dls_scanner) return;

    Info_new( __func__, LOG_INFO, domain, "'%s': Copy to disk OK. Parsing in progress", tech_id );
/*********************************************************** Parsing **********************************************************/
    FILE *rc = fopen( source, "r" );
    if (!rc)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Open source File Error", tech_id );
       Json_node_add_string ( PluginNode, "compil_error", "Open source file error" );
       End_scanner ( Dls_scanner );
       return;
     }

    Emettre( Dls_scanner->scan_instance, " #include <Module_dls.h>\n" );
    Emettre( Dls_scanner->scan_instance, " #include <math.h>\n" );
/*------------------------------------- Création des mnemoniques permanents -----------------------------------------------*/
    GList *options;
    options = New_option_chaine ( NULL, T_LIBELLE, "Statut de Synthèse de la communication du module" );
    New_alias_systeme ( Dls_scanner->scan_instance, "COMM", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des défauts et alarmes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSA_OK", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des défauts fixes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSA_DEFAUT_FIXE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des défauts" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSA_DEFAUT", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des alarmes fixes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSA_ALARME_FIXE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des alarmes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSA_ALARME", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Statut de la veille" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSB_VEILLE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des alertes fixes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSB_ALERTE_FIXE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des alertes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSB_ALERTE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des dangers et dérangements" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSP_OK", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des dérangements fixes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSP_DERANGEMENT_FIXE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des dérangements" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSP_DERANGEMENT", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des dangers fixes" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSP_DANGER_FIXE", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Synthèse des dangers" );
    New_alias_systeme ( Dls_scanner->scan_instance, "MEMSSP_DANGER", T_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Acquit via synoptique" );
    New_alias_systeme ( Dls_scanner->scan_instance, "OSYN_ACQUIT", T_DIGITAL_INPUT, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Communication OK" );
    options = New_option_entier ( options, T_TYPE, MSG_ETAT );
    New_alias_systeme ( Dls_scanner->scan_instance, "MSG_COMM_OK", T_MSG, options );

    options = New_option_chaine ( NULL, T_LIBELLE, "Communication Hors Service" );
    options = New_option_entier ( options, T_TYPE, MSG_DEFAUT );
    New_alias_systeme ( Dls_scanner->scan_instance,  "MSG_COMM_HS", T_MSG, options );

    DlsScanner_restart(rc, Dls_scanner->scan_instance );
    DlsScanner_set_lineno( 1, Dls_scanner->scan_instance );                                       /* reset du numéro de ligne */
    DlsScanner_parse( Dls_scanner->scan_instance );                                              /* Parsing du fichier source */

    struct tm *temps;
    time_t ltime;
    gchar date[32];
    time(&ltime);
    temps = localtime( (time_t *)&ltime );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else { g_snprintf(date, sizeof(date), "Erreur"); }

    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine),
              "/*******************************************************/\n"
              " gchar *version (void)\n"
              "  { return(\"%s - %s\"); \n  }\n ", ABLS_API_VERSION, date );
    Emettre( Dls_scanner->scan_instance, chaine );                                                    /* Ecriture du prologue */
    fclose(rc);

    if (Dls_scanner->nbr_erreur)
     { Json_node_add_bool   ( PluginNode, "compil_status", TRUE );                             /* compil ok but errors in dls */
       Json_node_add_string ( PluginNode, "errorlog", Dls_scanner->Error );
       Json_node_add_int    ( PluginNode, "error_count", Dls_scanner->nbr_erreur );
       Info_new( __func__, LOG_INFO, domain, "'%s': %d errors found", tech_id, Dls_scanner->nbr_erreur );
       End_scanner ( Dls_scanner );
       return;
     }

    Info_new( __func__, LOG_INFO, domain, "'%s': No parsing error, starting mnemonique import", tech_id );

    if ( Json_get_bool ( PluginNode, "enable" ) == FALSE )
     { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: Plugin '%s' is not enabled", tech_id ); }

/******************** Creation de la fonction de mapping et preparation des listes d'acronymes utilisés ***********************/
    Emettre( Dls_scanner->scan_instance, "/*******************************************************/\n"
                                         " void remap_all_alias ( struct DLS_TO_PLUGIN *vars )\n"
                                         "  {\n");
    liste = Dls_scanner->Alias;                                 /* Libération des alias, et remonté d'un Warning si il y en a */
    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if ( alias->used == FALSE )
        { gboolean exception = FALSE;                                                   /* Liste des exceptions au "not used" */
          if ( alias->classe == T_VISUEL && !strcasecmp ( Get_option_chaine ( alias->options, T_FORME, "" ), "comment" ) ) exception = TRUE;
          if ( alias->classe == T_VISUEL && !strcasecmp ( Get_option_chaine ( alias->options, T_FORME, "" ), "encadre" ) ) exception = TRUE;
          if ( alias->classe == T_VISUEL )                                 /* Création du bit de CLIC associé a chaque visuel */
           { gchar chaine[128];
             g_snprintf ( chaine, sizeof(chaine), "%s_CLIC", alias->acronyme );
             struct ALIAS *clic = Get_local_alias ( Dls_scanner->scan_instance, alias->tech_id, chaine );
             if ( clic && clic->used == TRUE ) exception = TRUE;
           }
          if (!exception) Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s not used", alias->acronyme );
        }

       gchar chaine[256];
       switch ( alias->classe )
        { case T_BUS:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_BISTABLE:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_BI(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_MONOSTABLE:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_MONO(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_DIGITAL_INPUT:
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, NULL );
             if (libelle && alias->systeme==FALSE)
              {  Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'libelle' sera bientot interdit ",
                                      alias->tech_id, alias->acronyme );
              }
             g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_DI(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_DIGITAL_OUTPUT:
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, NULL );
             if (libelle && alias->systeme==FALSE)
              { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'libelle' sera bientot interdit ",
                                     alias->tech_id, alias->acronyme );
              }
             g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_DO(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_ANALOG_OUTPUT:
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, NULL );
             if (libelle && alias->systeme==FALSE)
              { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'libelle' sera bientot interdit ",
                                     alias->tech_id, alias->acronyme );
              }
             gchar *unite   = Get_option_chaine( alias->options, T_UNITE, NULL );
             if (unite && alias->systeme==FALSE)
              { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'unite' sera bientot interdit ",
                                     alias->tech_id, alias->acronyme );
              }
             g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_AO(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_ANALOG_INPUT:
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, NULL );
             if (libelle && alias->systeme==FALSE)
              { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'libelle' sera bientot interdit ",
                                     alias->tech_id, alias->acronyme );
              }
             gchar *unite   = Get_option_chaine( alias->options, T_UNITE, NULL );
             if (unite && alias->systeme==FALSE)
              { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s:%s : 'unite' sera bientot interdit ",
                                     alias->tech_id, alias->acronyme );
              }
             g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_AI(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_TEMPO:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_TEMPO(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_HORLOGE:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_HORLOGE(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_REGISTRE:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_REGISTRE(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_WATCHDOG:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_WATCHDOG(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_VISUEL:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_VISUEL(\"%s\", \"%s\");\n",
                            alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_CPT_IMP:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_CI(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_CPT_H:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_CH(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          case T_MSG:
           { g_snprintf ( chaine, sizeof(chaine), "_%s_%s = Dls_data_lookup_MESSAGE(\"%s\", \"%s\");\n",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
             Emettre ( Dls_scanner->scan_instance, chaine );
             break;
           }
          default : Emettre ( Dls_scanner->scan_instance, "/*error*/\n" );
        }
       liste = liste->next;
     }
    Emettre( Dls_scanner->scan_instance, "}\n" );

/***************************************************** Initialisation des visuels *********************************************/
    Emettre( Dls_scanner->scan_instance, "/*******************************************************/\n"
                                         " void init_visuels ( struct DLS_TO_PLUGIN *vars )\n"
                                         "  {\n");
    liste = Dls_scanner->Alias;                                                                        /* Pour tous les alias */
    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if (alias->classe == T_VISUEL && !strcmp(alias->tech_id, tech_id))
        { gchar *mode    = Get_option_chaine ( alias->options, T_MODE, "default" );
          gchar *couleur = Get_option_chaine ( alias->options, T_COLOR, "black" );
          gint   cligno  = Get_option_entier ( alias->options, CLIGNO, 0 );
          gint   noshow  = Get_option_entier ( alias->options, T_NOSHOW, 0 );
          gint   disable = Get_option_entier ( alias->options, T_DISABLE, 0 );
          gchar *libelle = Get_option_chaine ( alias->options, T_LIBELLE, "pas de libellé" );

          g_snprintf ( chaine, sizeof(chaine), "Dls_data_set_VISUEL( vars, _%s_%s, \"%s\", \"%s\", 0.0, %d, %d, \"%s\", %d );\n",
                       alias->tech_id, alias->acronyme, mode, couleur, cligno, noshow, libelle, disable );
          Emettre ( Dls_scanner->scan_instance, chaine );
        }
       liste = liste->next;
     }
    Emettre( Dls_scanner->scan_instance, "}\n/*** EOF ***/" );

/*--------------------------------------- Suppression des mnemoniques non utilisés -------------------------------------------*/
    DB_Write ( domain, "DELETE FROM mnemos_MONO     WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_BI       WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_AI       WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_AO       WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_DI       WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_DO       WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_REGISTRE WHERE used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_TEMPO    WHERE used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_CI       WHERE used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_CH       WHERE used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM msgs            WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_HORLOGE  WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_WATCHDOG WHERE deletable=1 AND used=0 AND tech_id='%s' ", tech_id );
    DB_Write ( domain, "DELETE FROM mnemos_VISUEL   WHERE tech_id='%s' AND used=0", tech_id );
    DB_Write ( domain, "DELETE FROM syns_motifs     WHERE dls_id=%d AND used=0", Json_get_int ( PluginNode, "dls_id" ) );

/*---------------------------------------------------- Erase old mapping -----------------------------------------------------*/
    DB_Write ( domain, "UPDATE mappings SET tech_id=NULL, acronyme=NULL WHERE tech_id='%s' "
                       " AND acronyme NOT IN (SELECT acronyme FROM dictionnaire WHERE tech_id='%s') ",
                       tech_id, tech_id );

/*-------------------------------------- Fin de traduction sans erreur + import mnemo ok -------------------------------------*/
    gint compil_time = Global.Top - compil_top;
    Json_node_add_int    ( PluginNode, "warning_count", Dls_scanner->nbr_erreur );
    Json_node_add_string ( PluginNode, "errorlog",      Dls_scanner->Error );
    Json_node_add_bool   ( PluginNode, "compil_status", TRUE );
    Json_node_add_int    ( PluginNode, "compil_time",   compil_time );
    Json_node_add_string ( PluginNode, "codec",         Dls_scanner->Buffer );                     /* Sauvegarde dans le Json */
    End_scanner ( Dls_scanner );
    Info_new( __func__, LOG_NOTICE, domain, "'%s': Compiled in %03.1fs", tech_id, compil_time/10.0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
