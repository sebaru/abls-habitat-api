/******************************************************************************************************************************/
/* TraductionDLS/Interp.c          Interpretation du langage DLS                                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                      dim 05 avr 2009 12:47:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Interp.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
    gint taille = strlen(chaine)+1;
    if ( Dls_scanner->buffer_used + taille > Dls_scanner->buffer_size )
     { gint new_taille = Dls_scanner->buffer_size + taille;
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
    Info_new( __func__, LOG_DEBUG, Dls_scanner->domain, "Ligne %d : %s", DlsScanner_get_lineno(scan_instance), chaine );
    memcpy ( Dls_scanner->Buffer + Dls_scanner->buffer_used, chaine, taille );                   /* Recopie du bout de buffer */
    Dls_scanner->buffer_used += taille;
  }
/******************************************************************************************************************************/
/* DlsScanner_error: Appellé par le scanner en cas d'erreur de syntaxe (et non une erreur de grammaire !)                     */
/* Entrée : la chaine source de l'erreur de syntaxe                                                                           */
/* Sortie : appel de la fonction Emettre_erreur_new en backend                                                                */
/******************************************************************************************************************************/
 int DlsScanner_error ( void *scan_instance ,char *s )
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

    Info_new( __func__, LOG_ERR, Dls_scanner->domain, "'%s': Ligne %04d : %s",
              Json_get_string ( Dls_scanner->PluginNode, "tech_id" ),
              DlsScanner_get_lineno(scan_instance), chaine );
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
       g_free(chaine);
       return(options);
     }

    option->token        = token;
    option->token_classe = T_CHAINE;
    option->chaine       = chaine;
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
 static struct CONDITION *New_condition_bi( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_BI ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree: Prepare la chaine de caractere associée à la condition, en respectant les options                    */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_entree( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_sortie_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_sortie_ana( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_mono: Prepare la chaine de caractere associée à la condition, en respectant les options                      */
/* Entrées: l'alias du monostable et sa liste d'options                                                                       */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_mono( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_MONO ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
   return(condition);
 }
/******************************************************************************************************************************/
/* New_condition_tempo: Prepare la chaine de caractere associée à la condition, en respectant les options                     */
/* Entrées: l'alias de la temporisatio et sa liste d'options                                                                  */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_tempo( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    g_snprintf( condition->chaine, condition->taille, "%sDls_data_get_tempo ( \"%s\", \"%s\", &_%s_%s )",
                (barre==1 ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_horloge( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (!barre)
     { g_snprintf( condition->chaine, condition->taille, "Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
   return(condition);
 }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_WATCHDOG( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( TRUE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);
    if (!barre)
     { g_snprintf( condition->chaine, condition->taille, "Dls_data_get_WATCHDOG ( \"%s\", \"%s\", &_%s_%s )",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_WATCHDOG ( \"%s\", \"%s\", &_%s_%s )",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
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
 static struct CONDITION *New_condition_entree_ana( int barre, struct ALIAS *alias, GList *options )
  {
    gint in_range = Get_option_entier ( options, T_IN_RANGE, 0 );

    struct CONDITION *condition = New_condition( (in_range ? TRUE : FALSE), 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (in_range==1)
     { if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s)",
                              alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
             else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s)",
                              alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
       return(condition);
     }
    if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
          else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_registre( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_REGISTRE(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
          else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_REGISTRE(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_CI( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
          else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_CI(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
    return(condition);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static struct CONDITION *New_condition_CH( int barre, struct ALIAS *alias, GList *options )
  { struct CONDITION *condition = New_condition( FALSE, 256 ); /* 10 caractères max */
    if (!condition) return(NULL);

    if (barre) g_snprintf( condition->chaine, condition->taille, "!Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
          else g_snprintf( condition->chaine, condition->taille, "Dls_data_get_CH(\"%s\",\"%s\",&_%s_%s)",
                           alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
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
     { case MNEMO_TEMPO :     return ( New_condition_tempo( barre, alias, options ) );
       case MNEMO_ENTREE_TOR: return ( New_condition_entree( barre, alias, options ) );
       case MNEMO_BISTABLE:   return ( New_condition_bi( barre, alias, options ) );
       case MNEMO_MONOSTABLE: return ( New_condition_mono( barre, alias, options ) );
       case MNEMO_HORLOGE:    return ( New_condition_horloge( barre, alias, options ) );
       case MNEMO_WATCHDOG:   return ( New_condition_WATCHDOG( barre, alias, options ) );
       case MNEMO_ENTREE_ANA: return ( New_condition_entree_ana( barre, alias, options ) );
       case MNEMO_SORTIE_ANA: return ( New_condition_sortie_ana( barre, alias, options ) );
       case MNEMO_REGISTRE:   return ( New_condition_registre( barre, alias, options ) );
       case MNEMO_CPT_IMP:    return ( New_condition_CI( barre, alias, options ) );
       case MNEMO_CPTH:       return ( New_condition_CH( barre, alias, options ) );
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
    if ( input->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : input must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *consigne = Get_option_alias ( options, T_CONSIGNE );
    if (!consigne)
     { Emettre_erreur_new ( scan_instance, "PID : consigne unknown. Select one R." );
       return(g_strdup("0"));
     }
    if ( consigne->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : consigne must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kp = Get_option_alias ( options, T_KP );
    if (!kp)
     { Emettre_erreur_new ( scan_instance, "PID : kp. Select one R." );
       return(g_strdup("0"));
     }
    if ( kp->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kp must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *ki = Get_option_alias ( options, T_KD );
    if (!ki)
     { Emettre_erreur_new ( scan_instance, "PID : ki. Select one R." );
       return(g_strdup("0"));
     }
    if ( ki->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : ki must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kd = Get_option_alias ( options, T_KI );
    if (!kd)
     { Emettre_erreur_new ( scan_instance, "PID : kd. Select one R." );
       return(g_strdup("0"));
     }
    if ( kd->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kd must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_min = Get_option_alias ( options, T_MIN );
    if (!output_min)
     { Emettre_erreur_new ( scan_instance, "PID : output_min. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_min->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output_min must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_max = Get_option_alias ( options, T_MAX );
    if (!output_max)
     { Emettre_erreur_new ( scan_instance, "PID : output_max. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_max->classe != MNEMO_REGISTRE )
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
    if ( ! (input->classe == MNEMO_REGISTRE /*|| input->classe == MNEMO_ENTREE_ANA*/ ) )
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
 struct ACTION *New_action_msg( void *scan_instance, struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    if (strcasecmp ( alias->tech_id, Json_get_string ( Dls_scanner->PluginNode, "tech_id" ) ) )
     { Emettre_erreur_new ( scan_instance, "Setting an external MSG (%s:%s) is forbidden",  alias->tech_id, alias->acronyme );
       return(NULL);
     }

    if (alias->used!=1)
     { Emettre_erreur_new ( scan_instance, "Message %s could not be used more than once",  alias->acronyme );
       return(NULL);
     }

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    gint update = Get_option_entier ( options, T_UPDATE, 0 );
    gint groupe = Get_option_entier ( options, T_GROUPE, 0 );

    if (groupe>0)
     { g_snprintf( action->alors, taille, "   Dls_data_set_MSG_groupe ( vars, \"%s\", \"%s\", &_%s_%s, %d );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, groupe );
     }
    else
     { g_snprintf( action->alors, taille, "   Dls_data_set_MSG ( vars, \"%s\", \"%s\", &_%s_%s, %s, TRUE );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, (update ? "TRUE" : "FALSE") );
       g_snprintf( action->sinon, taille, "   Dls_data_set_MSG ( vars, \"%s\", \"%s\", &_%s_%s, %s, FALSE );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, (update ? "TRUE" : "FALSE") );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_sortie: Prepare la structure ACTION associée à l'alias en paramètre                                             */
/* Entrées: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associée                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_sortie( struct ALIAS *alias, int barre, GList *options )
  { struct ACTION *action = New_action();
    gint taille = 128;
    action->alors = New_chaine( taille );
    if ( (!barre) )
         { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_digital_output: Prepare la structure ACTION associée à l'alias en paramètre                                     */
/* Entrées: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associée                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_digital_output( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    gint taille = 128;

    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    g_snprintf( action->sinon, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_vars_mono: Prepare une struct action avec une commande SM                                                       */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_vars_mono( gchar *nom )
  { struct ACTION *action;
    int taille;

    taille = strlen(nom)+5;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "%s=1;", nom );
    g_snprintf( action->sinon, taille, "%s=0;", nom );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_mono( struct ALIAS *alias )
  { struct ACTION *action;
    int taille;

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = NULL;

    g_snprintf( action->alors, taille, "   Dls_data_set_MONO ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_h( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    gint reset = Get_option_entier ( options, T_RESET, 0 );
    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CH ( vars, \"%s\", \"%s\", &_%s_%s, TRUE, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CH ( vars, \"%s\", \"%s\", &_%s_%s, FALSE, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_imp( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    gint reset = Get_option_entier ( options, T_RESET, 0 );
    gint ratio = Get_option_entier ( options, T_RATIO, 1 );

    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CI ( vars, \"%s\", \"%s\", &_%s_%s, TRUE, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset, ratio );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CI ( vars, \"%s\", \"%s\", &_%s_%s, FALSE, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset, ratio );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_WATCHDOG: Prepare une struct action pour une action de type WATCHDOG                                            */
/* Entrées: l'alias source, et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_WATCHDOG( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    struct ALIAS *alias_consigne = Get_option_alias ( options, T_CONSIGNE );
    if (alias_consigne)
     { gint taille = 512;
       action = New_action();
       action->alors = New_chaine( taille );

       g_snprintf( action->alors, taille,
                   "   Dls_data_set_WATCHDOG ( vars, \"%s\", \"%s\", &_%s_%s, \n"
                   "                           Dls_data_get_REGISTRE ( \"%s\", \"%s\", &_%s_%s ) );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                   alias_consigne->tech_id, alias_consigne->acronyme, alias_consigne->tech_id, alias_consigne->acronyme
                 );
       return(action);
     }

    gint consigne = Get_option_entier ( options, T_CONSIGNE, 600 );
    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_WATCHDOG ( vars, \"%s\", \"%s\", &_%s_%s, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, consigne );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_registre: Prepare une struct action avec une commande registre                                                  */
/* Entrées: l'alias associé et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_REGISTRE( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_REGISTRE ( vars, \"%s\", \"%s\", &_%s_%s, local_result );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_AO: Prepare une struct action avec une commande analog Output                                                   */
/* Entrées: l'alias associé et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_AO( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_AO ( vars, \"%s\", \"%s\", &_%s_%s, local_result );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_visuel: Prepare une struct action avec une commande SI                                                           */
/* Entrées: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_visuel( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, mode;

    gchar *mode_string = Get_option_chaine ( options, T_MODE, NULL );
    if (mode_string == NULL) mode = Get_option_entier ( options, T_MODE, 0   );
    gchar *couleur = Get_option_chaine ( options, T_COLOR, "black" );
    gint   cligno  = Get_option_entier ( options, CLIGNO, 0 );
    gchar *libelle = Get_option_chaine ( options, T_LIBELLE, "pas de libellé" );
    taille = 768;
    action = New_action();
    action->alors = New_chaine( taille );

    if (mode_string==NULL)
     { g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, \"%d\", \"%s\", %d, \"%s\" );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode, couleur, cligno, libelle );
     }
    else
     { g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, \"%s\", \"%s\", %d, \"%s\" );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode_string, couleur, cligno, libelle );
     }

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
 struct ACTION *New_action_bus( struct ALIAS *alias, GList *options )
  { struct ACTION *result;
    gchar *option_chaine;
    gint taille;

    gchar *target_tech_id = Get_option_chaine ( options, T_TECH_ID, "*" );

    JsonNode *RootNode = Json_node_create ();
    option_chaine = Get_option_chaine ( options, T_TAG, "PING" );
    if (option_chaine) Json_node_add_string ( RootNode, "tag", option_chaine );

    option_chaine = Get_option_chaine ( options, T_TARGET, NULL );
    if (option_chaine) Json_node_add_string ( RootNode, "target", option_chaine );

    gchar *json_buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
    gchar *normalized_buf = Normaliser_chaine_for_dls ( json_buf );
    g_free(json_buf);

    result = New_action();
    taille = 256+strlen(target_tech_id)+strlen(json_buf);
    result->alors = New_chaine( taille );
    g_snprintf( result->alors, taille,
                 "   Dls_data_set_bus ( \"%s\", \"%s\", &_%s_%s, \"%s\", \"%s\" );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                target_tech_id, normalized_buf );
    g_free(normalized_buf);
    return(result);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_tempo( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, daa, dma, dMa, dad, random;

    daa    = Get_option_entier ( options, T_DAA, 0 );
    dma    = Get_option_entier ( options, T_DMINA, 0 );
    dMa    = Get_option_entier ( options, T_DMAXA, 0 );
    dad    = Get_option_entier ( options, T_DAD, 0 );
    random = Get_option_entier ( options, T_RANDOM, 0 );

    action = New_action();
    taille = 256;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille,
                "   Dls_data_set_tempo ( vars, \"%s\", \"%s\", &_%s_%s, 1, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille,
                "   Dls_data_set_tempo ( vars, \"%s\", \"%s\", &_%s_%s, 0, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bi( struct ALIAS *alias, gint barre )
  { struct ACTION *action;
    int taille;

    gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    if (groupe == 0)
     { g_snprintf( action->alors, taille, "   Dls_data_set_BI ( vars, \"%s\", \"%s\", &_%s_%s, %s );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, (barre ? "FALSE" : "TRUE") );
     }
    else if(barre)
     { g_snprintf( action->alors, taille, "   Dls_data_set_BI ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( action->alors, taille, "   Dls_data_set_BI_groupe ( vars, \"%s\", \"%s\", &_%s_%s, %d );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, groupe );
     }

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
/* New_alias_dependance_DI: Creer un nouvel Alias de depandences                                                              */
/* Entrées: le tech_id/acronyme de l'alias                                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void New_alias_dependance_DI ( void *scan_instance, gchar *tech_id, gchar *acronyme, gchar *libelle )
  { if ( ! Get_local_alias ( scan_instance, tech_id, acronyme ) )                                                      /* Si pas déjà défini */
     { GList *ss_options = New_option_chaine ( NULL, T_LIBELLE, g_strdup(libelle) );
       struct ALIAS *alias_dep = New_alias ( scan_instance, tech_id, acronyme, MNEMO_ENTREE_TOR, ss_options );
       if (alias_dep) alias_dep->used = 1;                         /* Par défaut, on considère qu'une dependance est utilisée */
     }
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le tech_id/Acronyme de l'alias                                                                                    */
/* Sortie: la structure, ou FALSE si erreur                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *New_alias ( void *scan_instance, gchar *tech_id, gchar *acronyme, gint classe, GList *options )
  { struct ALIAS *alias;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );

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

    if (!strcmp(alias->tech_id, plugin_tech_id))     /* Pour tous les alias locaux, on créé une entrée en base de données */
     { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, "no libelle" );
       switch(alias->classe)
        { case MNEMO_BUS:
             break;
          case MNEMO_BISTABLE:
           { gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
             Mnemo_auto_create_BI ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle, groupe );
             break;
           }
          case MNEMO_MONOSTABLE:
           { Mnemo_auto_create_MONO ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_ENTREE_TOR:
           { Mnemo_auto_create_DI ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_SORTIE_TOR:
           { Mnemo_auto_create_DO ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_SORTIE_ANA:
           { Mnemo_auto_create_AO ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_ENTREE_ANA:
           { Mnemo_auto_create_AI ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme,
                                    Get_option_chaine( alias->options, T_LIBELLE, NULL ),
                                    Get_option_chaine( alias->options, T_UNITE, NULL ) );
             break;
           }
          case MNEMO_TEMPO:
           { Mnemo_auto_create_TEMPO ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_HORLOGE:
           { Mnemo_auto_create_HORLOGE ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_REGISTRE:
           { Mnemo_auto_create_REGISTRE ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle,
                                          Get_option_chaine( alias->options, T_UNITE, "no unit" ) );
             break;
           }
          case MNEMO_WATCHDOG:
           { Mnemo_auto_create_WATCHDOG ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_VISUEL:
           { gchar *forme   = Get_option_chaine( alias->options, T_FORME, NULL );
             gchar *couleur = Get_option_chaine( alias->options, T_COLOR, "black" );
             gchar *mode    = Get_option_chaine( alias->options, T_MODE, "default" );
             if (forme)
              { gchar ss_acronyme[64];
                g_snprintf( ss_acronyme, sizeof(ss_acronyme), "%s_CLIC", acronyme );
                New_alias_dependance_DI ( scan_instance, tech_id, ss_acronyme, "Clic sur le visuel depuis l'IHM" );
                Mnemo_auto_create_VISUEL ( Dls_scanner->domain, Dls_scanner->PluginNode, alias->acronyme, libelle, forme, mode, couleur );
                Synoptique_auto_create_MOTIF ( Dls_scanner->domain, Dls_scanner->PluginNode, alias->tech_id, alias->acronyme );
              }
             break;
           }
          case MNEMO_CPT_IMP:
           { Mnemo_auto_create_CI ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle,
                                    Get_option_chaine ( alias->options, T_UNITE, "fois" ),
                                    Get_option_double ( alias->options, T_MULTI, 1.0 ) );
             break;
           }
          case MNEMO_CPTH:
           { Mnemo_auto_create_CH ( Dls_scanner->domain, plugin_tech_id, alias->acronyme, libelle );
             break;
           }
          case MNEMO_MSG:
           { gint type   = Get_option_entier ( alias->options, T_TYPE, MSG_ETAT );
             gint groupe = Get_option_entier ( alias->options, T_GROUPE, 0 );
             Mnemo_auto_create_MSG ( Dls_scanner->domain, TRUE, plugin_tech_id, alias->acronyme, libelle, type, groupe );
             break;
           }
        }
     }
    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), " static gpointer _%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
    Emettre( Dls_scanner->scan_instance, chaine );

    return(alias);
  }
/******************************************************************************************************************************/
/* New_alias: Allouecomp une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 static struct ALIAS *New_alias_permanent ( void *scan_instance, gchar *tech_id, gchar *acronyme, gint classe, GList *options )
  { struct ALIAS *alias = New_alias ( scan_instance, tech_id, acronyme, classe, options );
    if (alias) { alias->used=1;}                                                      /* Un alias permanent est toujours used */
    return(alias);
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
    if (!result) { Info_new( __func__, LOG_DEBUG, NULL, "'%s:%s' not found in DICO", alias->tech_id, alias->acronyme ); return(NULL); }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "VISUEL" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_VISUEL, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "DI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_ENTREE_TOR, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "DO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_DIGITAL_OUTPUT, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "AI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_ENTREE_ANA, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "AO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_SORTIE_ANA, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "MONO" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_MONOSTABLE, options );
       json_node_unref ( result );
     }
    else if ( Json_has_member ( result, "classe" ) && !strcmp ( Json_get_string ( result, "classe" ), "BI" ) )
     { alias = New_alias ( scan_instance, tech_id, acronyme, MNEMO_BISTABLE, options );
       json_node_unref ( result );
     }
    else
     { json_node_unref ( result );
       result = Rechercher_DICO ( Dls_scanner->domain, "SYS", acronyme );
       if (!result) { return(NULL); }

/*       if ( Json_has_member ( result, "classe_int" ) && Json_get_int ( result, "classe_int" ) != -1 )
        { alias = New_alias ( scan_instance, "SYS", acronyme, Json_get_int ( result, "classe_int" ), options ); }
       else { json_node_unref(result); return(NULL); }                             /* Si pas trouvé en externe, retourne NULL */
     }

    if (alias)                                                                 /* Si trouvé, on considère que le bit est used */
     { alias->used     = 1;
       Info_new( __func__, LOG_DEBUG, NULL, "'%s:%s'", alias->tech_id, alias->acronyme );
     }
    else { Info_new( __func__, LOG_DEBUG, NULL, "'%s:%s' new_alias failed", tech_id, acronyme ); }
    return(alias);
  }
/******************************************************************************************************************************/
/* Get_local_alias: Recherche un alias donné en paramètre                                                                           */
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
       if (!strcmp(alias->acronyme, acronyme) &&
            ( !strcmp(alias->tech_id,tech_id) || !strcmp(alias->tech_id,"SYS") )
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
    g_free(alias->tech_id);
    g_free(alias->acronyme);
    g_free(alias);
  }
/******************************************************************************************************************************/
/* Add_csv: Ajoute un membre dans une liste type CSV                                                                          */
/* Entrées: la chaine actuelle, la chaine a ajouter                                                                           */
/* Sortie: la nouvelle chaine complétée                                                                                       */
/******************************************************************************************************************************/
 static gchar *Add_csv ( gchar *source, gchar *to_add )
  { if (!source) return ( g_strconcat( "'", to_add, "'", NULL ) );
    gchar *new_source = g_strconcat ( source, ", '", to_add, "'", NULL );
    g_free(source);
    return(new_source);
  }
/******************************************************************************************************************************/
/* Add_alias_csv: Ajoute un alias dans une liste type CSV                                                                     */
/* Entrées: la chaine actuelle, la chaine a ajouter                                                                           */
/* Sortie: la nouvelle chaine complétée                                                                                       */
/******************************************************************************************************************************/
 static gchar *Add_alias_csv ( gchar *source, gchar *tech_id, gchar *acronyme )
  { if (!tech_id || !acronyme) return(source);
    if (!source)
     { return ( g_strconcat( "'", tech_id, ":", acronyme, "'", NULL ) ); }

    gchar *new_source = g_strconcat ( source, ", '", tech_id, ":", acronyme, "'", NULL );
    g_free(source);
    return(new_source);
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
    struct DLS_TRAD *scanner = g_try_malloc0 ( sizeof ( struct DLS_TRAD ) );
    if (!scanner)
     { Info_new( __func__, LOG_ERR, domain, "'%s': DLS_TRAD memory error", tech_id );
       Json_node_add_string ( PluginNode, "compil_error_log", "Memory Scanner Error" );
       return(NULL);
     }
    scanner->PluginNode = PluginNode;
    scanner->domain = domain;
    scanner->nbr_erreur = 0;                                                          /* Au départ, nous n'avons pas d'erreur */

    scanner->buffer_size = 1024;
    scanner->Buffer = g_try_malloc0( scanner->buffer_size );                             /* Initialisation du buffer resultat */
    if (!scanner->Buffer)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Not enought memory for buffer", tech_id );
       Json_node_add_string ( PluginNode, "compil_error_log", "Memory error for buffer" );
       End_scanner ( scanner );
       return(NULL);
     }
    scanner->buffer_used = 0;

    scanner->Error = g_try_malloc0( 1 );                                                 /* Initialisation du buffer resultat */
    if (!scanner->Error)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Not enought memory for ErrorBuffer", tech_id );
       Json_node_add_string ( PluginNode, "compil_status", "Memory error for ErrorBuffer" );
       End_scanner ( scanner );
       return(NULL);
     }

    DlsScanner_lex_init (&scanner->scan_instance);
    DlsScanner_debug = Json_get_bool ( PluginNode, "debug" );
    DlsScanner_set_extra( (void *)scanner, scanner->scan_instance );

    return(scanner);
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

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gchar *tech_id     = Json_get_string ( PluginNode, "tech_id" );
    Json_node_add_int ( PluginNode, "compil_error_count",   0 );
    Json_node_add_int ( PluginNode, "compil_warning_count", 0 );
    DB_Write ( domain, "UPDATE dls SET nbr_compil=nbr_compil+1 WHERE tech_id='%s'", tech_id );

/************************************************ Descend le sourcecode sur disque ********************************************/
    g_snprintf( source, sizeof(source), "/tmp/%s-%s.dls", domain_uuid, tech_id );
    unlink ( source );
    gint fd_source = open( source, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd_source<0)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Source creation failed %s (%s)", tech_id, source, strerror(errno) );
       Json_node_add_string ( PluginNode, "compil_status", "Source creation failed" );
       return;
     }
    gchar *sourcecode_to_write = Json_get_string ( PluginNode, "sourcecode" );
    if (sourcecode_to_write) write ( fd_source, sourcecode_to_write, strlen (sourcecode_to_write) );
    close(fd_source);

/************************************************ Charge un scanner ***********************************************************/
    struct DLS_TRAD *Dls_scanner = New_scanner ( domain, PluginNode );
    if (!Dls_scanner) return;

    Info_new( __func__, LOG_INFO, domain, "'%s': Parsing in progress", tech_id );

/*********************************************************** Parsing **********************************************************/
    FILE *rc = fopen( source, "r" );
    if (!rc)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Open source File Error", tech_id );
       Json_node_add_string ( PluginNode, "compil_status", "Open source file error" );
       End_scanner ( Dls_scanner );
       return;
     }

    Emettre( Dls_scanner->scan_instance, " #include <Module_dls.h>\n" );

/*------------------------------------- Création des mnemoniques permanents -----------------------------------------------*/
    GList *options;
    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Statut de Synthèse de la communication du module"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "COMM", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des défauts et alarmes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSA_OK", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des défauts fixes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSA_DEFAUT_FIXE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des défauts"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSA_DEFAUT", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des alarmes fixes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSA_ALARME_FIXE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des alarmes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSA_ALARME", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Statut de la veille"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSB_VEILLE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des alertes fixes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSB_ALERTE_FIXE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des alertes fugitives"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSB_ALERTE_FUGITIVE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des alertes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSB_ALERTE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des dangers et dérangements"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSP_OK", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des dérangements fixes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSP_DERANGEMENT_FIXE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des dérangements"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSP_DERANGEMENT", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des dangers fixes"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSP_DANGER_FIXE", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Synthèse des dangers"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MEMSSP_DANGER", MNEMO_MONOSTABLE, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Acquit via synoptique"));
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "OSYN_ACQUIT", MNEMO_ENTREE_TOR, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Communication OK"));
    options = New_option_entier ( options, T_TYPE, MSG_ETAT );
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MSG_COMM_OK", MNEMO_MSG, options );

    options = New_option_chaine ( NULL, T_LIBELLE, g_strdup("Communication Hors Service"));
    options = New_option_entier ( options, T_TYPE, MSG_DEFAUT );
    New_alias_permanent ( Dls_scanner->scan_instance, NULL, "MSG_COMM_HS", MNEMO_MSG, options );

    DlsScanner_restart(rc, Dls_scanner->scan_instance );
    DlsScanner_set_lineno( 1, Dls_scanner->scan_instance );                                        /* reset du numéro de ligne */
    DlsScanner_parse( Dls_scanner->scan_instance );                                               /* Parsing du fichier source */

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
              "  { return(\"%s - %s\"); \n  }\n /* EOF */", ABLS_API_VERSION, date );
    Emettre( Dls_scanner->scan_instance, chaine );                                                    /* Ecriture du prologue */
    fclose(rc);

    if (Dls_scanner->nbr_erreur)
     { Json_node_add_bool   ( PluginNode, "compil_status", TRUE );                             /* compil ok but errors in dls */
       Json_node_add_string ( PluginNode, "compil_error_log", Dls_scanner->Error );
       Json_node_add_int    ( PluginNode, "compil_error_count", Dls_scanner->nbr_erreur );
       Info_new( __func__, LOG_INFO, domain, "'%s': %d errors found", tech_id, Dls_scanner->nbr_erreur );
       End_scanner ( Dls_scanner );
       return;
     }

    Info_new( __func__, LOG_INFO, domain, "'%s': No parsing error, starting mnemonique import", tech_id );

/*----------------------------------------------- Prise en charge du peuplement de la database -------------------------------*/
    gchar *Liste_MONO = NULL, *Liste_BI = NULL, *Liste_DI = NULL, *Liste_DO = NULL, *Liste_AO = NULL, *Liste_AI = NULL;
    gchar *Liste_TEMPO = NULL, *Liste_HORLOGE = NULL, *Liste_REGISTRE = NULL, *Liste_WATCHDOG = NULL, *Liste_MESSAGE = NULL;
    gchar *Liste_CI = NULL, *Liste_CH = NULL;
    gchar *Liste_CADRANS = NULL, *Liste_MOTIF = NULL;
    liste = Dls_scanner->Alias;                                  /* Libération des alias, et remonté d'un Warning si il y en a */

    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if ( alias->used == FALSE &&
             ( ! ( alias->classe == MNEMO_VISUEL &&                                 /* Pas de warning pour les comments unused */
                   !strcasecmp ( Get_option_chaine ( alias->options, T_FORME, "" ), "comment" )
                 )
             )
          )
        { Emettre_erreur_new ( Dls_scanner->scan_instance, "Warning: %s not used", alias->acronyme ); }
/************************ Calcul des alias locaux pour préparer la suppression automatique ************************************/
       if (!strcmp(alias->tech_id, tech_id))
        {      if (alias->classe == MNEMO_BUS)        { }
          else if (alias->classe == MNEMO_MONOSTABLE) { Liste_MONO = Add_csv ( Liste_MONO, alias->acronyme ); }
          else if (alias->classe == MNEMO_BISTABLE)   { Liste_BI = Add_csv ( Liste_BI, alias->acronyme ); }
          else if (alias->classe == MNEMO_ENTREE_TOR) { Liste_DI = Add_csv ( Liste_DI, alias->acronyme ); }
          else if (alias->classe == MNEMO_SORTIE_TOR) { Liste_DO = Add_csv ( Liste_DO, alias->acronyme ); }
          else if (alias->classe == MNEMO_SORTIE_ANA) { Liste_AO = Add_csv ( Liste_AO, alias->acronyme ); }
          else if (alias->classe == MNEMO_ENTREE_ANA) { Liste_AI = Add_csv ( Liste_AI, alias->acronyme ); }
          else if (alias->classe == MNEMO_TEMPO)      { Liste_TEMPO = Add_csv ( Liste_TEMPO, alias->acronyme ); }
          else if (alias->classe == MNEMO_HORLOGE)    { Liste_HORLOGE = Add_csv ( Liste_HORLOGE, alias->acronyme ); }
          else if (alias->classe == MNEMO_REGISTRE)   { Liste_REGISTRE = Add_csv ( Liste_REGISTRE, alias->acronyme ); }
          else if (alias->classe == MNEMO_WATCHDOG)   { Liste_WATCHDOG = Add_csv ( Liste_WATCHDOG, alias->acronyme ); }
          else if (alias->classe == MNEMO_CPT_IMP)    { Liste_CI = Add_csv ( Liste_CI, alias->acronyme ); }
          else if (alias->classe == MNEMO_CPTH)       { Liste_CH = Add_csv ( Liste_CH, alias->acronyme ); }
          else if (alias->classe == MNEMO_MSG)        { Liste_MESSAGE = Add_csv ( Liste_MESSAGE, alias->acronyme ); }
          else if (alias->classe == MNEMO_VISUEL)
           { gchar *forme = Get_option_chaine( alias->options, T_FORME, NULL );
             if (forme) { Liste_MOTIF = Add_csv ( Liste_MOTIF, alias->acronyme ); }
           }
        }
/***************************************************** Création des visuels externes ******************************************/
       else if (alias->classe == MNEMO_VISUEL)                                   /* Création du LINK vers le visuel externe */
        { Synoptique_auto_create_MOTIF ( domain, Dls_scanner->PluginNode, alias->tech_id, alias->acronyme ); }
/***************************************************** Création des cadrans ***************************************************/
       gchar *cadran = Get_option_chaine( alias->options, T_CADRAN, NULL );
       if (cadran &&
            ( alias->classe == MNEMO_ENTREE_ANA ||
              alias->classe == MNEMO_REGISTRE ||
              alias->classe == MNEMO_CPTH ||
              alias->classe == MNEMO_CPT_IMP
            )
          )
        { gint default_decimal = 0;
          if (alias->classe == MNEMO_ENTREE_ANA || alias->classe == MNEMO_REGISTRE) default_decimal = 2;
          /*Synoptique_auto_create_CADRAN ( &Dls_plugin, alias->tech_id, alias->acronyme, cadran,
                                          Get_option_double ( alias->options, T_MIN, 0.0 ),
                                          Get_option_double ( alias->options, T_MAX, 100.0 ),
                                          Get_option_double ( alias->options, T_SEUIL_NTB, 5.0 ),
                                          Get_option_double ( alias->options, T_SEUIL_NB, 10.0 ),
                                          Get_option_double ( alias->options, T_SEUIL_NH, 90.0 ),
                                          Get_option_double ( alias->options, T_SEUIL_NTH, 05.0 ),
                                          default_decimal
                                        );*/
          Liste_CADRANS = Add_alias_csv ( Liste_CADRANS, alias->tech_id, alias->acronyme );
        }
       liste = liste->next;
     }
/*--------------------------------------- Suppression des mnemoniques non utilisés -------------------------------------------*/
    DB_Write ( domain, "DELETE FROM mnemos_MONO WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_MONO?Liste_MONO:"''") );
    if (Liste_MONO) g_free(Liste_MONO);

    DB_Write ( domain, "DELETE FROM mnemos_BI WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_BI?Liste_BI:"''") );
    if (Liste_BI) g_free(Liste_BI);

    DB_Write ( domain, "DELETE FROM mnemos_AI WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_AI?Liste_AI:"''") );
    if (Liste_AI) g_free(Liste_AI);

    DB_Write ( domain, "DELETE FROM mnemos_AO WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_AO?Liste_AO:"''") );
    if (Liste_AO) g_free(Liste_AO);

    DB_Write ( domain, "DELETE FROM mnemos_DI WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_DI?Liste_DI:"''") );
    if (Liste_DI) g_free(Liste_DI);

    DB_Write ( domain, "DELETE FROM mnemos_DO WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_DO?Liste_DO:"''") );
    if (Liste_DO) g_free(Liste_DO);

    DB_Write ( domain, "DELETE FROM mnemos_REGISTRE WHERE tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_REGISTRE?Liste_REGISTRE:"''") );
    if (Liste_REGISTRE) g_free(Liste_REGISTRE);

    DB_Write ( domain, "DELETE FROM mnemos_TEMPO WHERE tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_TEMPO?Liste_TEMPO:"''") );
    if (Liste_TEMPO) g_free(Liste_TEMPO);

    DB_Write ( domain, "DELETE FROM mnemos_CI WHERE tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_CI?Liste_CI:"''") );
    if (Liste_CI) g_free(Liste_CI);

    DB_Write ( domain, "DELETE FROM mnemos_CH WHERE tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_CH?Liste_CH:"''") );
    if (Liste_CH) g_free(Liste_CH);

    DB_Write ( domain, "DELETE FROM msgs WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_MESSAGE?Liste_MESSAGE:"''") );
    if (Liste_MESSAGE) g_free(Liste_MESSAGE);

    DB_Write ( domain, "DELETE FROM mnemos_HORLOGE WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_HORLOGE?Liste_HORLOGE:"''") );
    if (Liste_HORLOGE) g_free(Liste_HORLOGE);

    DB_Write ( domain, "DELETE FROM mnemos_WATCHDOG WHERE deletable=1 AND tech_id='%s' "
                       " AND acronyme NOT IN (%s)", tech_id, (Liste_WATCHDOG?Liste_WATCHDOG:"''")  );
    if (Liste_WATCHDOG) g_free(Liste_WATCHDOG);

    /*SQL_Write_new ( "DELETE FROM syns_cadrans WHERE dls_id='%d' AND CONCAT(tech_id,':',acronyme) NOT IN (%s)",
                    Dls_plugin.dls_id, (Liste_CADRANS ? Liste_CADRANS: "''" ) );*/
    if (Liste_CADRANS) g_free(Liste_CADRANS);

    DB_Write ( domain, "DELETE FROM mnemos_VISUEL WHERE tech_id='%s' "
                       " AND acronyme NOT IN ( %s )",
                       tech_id, (Liste_MOTIF?Liste_MOTIF:"''") );
    if (Liste_MOTIF) g_free(Liste_MOTIF);

/*-------------------------------------- Fin de traduction sans erreur + import mnemo ok -------------------------------------*/
    Json_node_add_int    ( PluginNode, "compil_line_number",   DlsScanner_get_lineno(Dls_scanner->scan_instance) );
    Json_node_add_int    ( PluginNode, "compil_warning_count", Dls_scanner->nbr_erreur );
    Json_node_add_string ( PluginNode, "codec",                Dls_scanner->Buffer );                /* Sauvegarde dans le Json */
    Json_node_add_string ( PluginNode, "compil_error_log",     Dls_scanner->Error );
    Json_node_add_bool   ( PluginNode, "compil_status", TRUE );
    End_scanner ( Dls_scanner );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
