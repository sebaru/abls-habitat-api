/******************************************************************************************************************************/
/* TraductionDLS/actions.c          Gestions des actions du langage DLS                                                       */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                                14.04.2025 05:15:04 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * actions.c
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
/* New_action_PID: Calcul un PID                                                                                              */
/* Entrées: la liste d'option associée au PID                                                                                 */
/* Sortie: la chaine de calcul DLS                                                                                            */
/******************************************************************************************************************************/
 struct ACTION *New_action_PID ( void *scan_instance, GList *options )
  { gint reset = Get_option_entier ( options, T_RESET, 0 );
    if (reset==1)                                                                                        /* PID en mode reset */
     { struct ALIAS *input = Get_option_alias ( options, T_INPUT );
       if (!input)
        { Emettre_erreur_new ( scan_instance, "PID : input is undefined. Select one input (R)." );
          return(NULL);
        }
       if ( ! (input->classe == T_REGISTRE) )
        { Emettre_erreur_new ( scan_instance, "PID : input must be R" );
          return(NULL);
        }

       struct ACTION *action = New_action();
       gint taille = 256;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "Dls_PID_reset ( vars, _%s_%s ); ", input->tech_id, input->acronyme );
       return(action);
     }

    struct ALIAS *input = Get_option_alias ( options, T_INPUT );                                        /* PID en mode calcul */
    if (!input)
     { Emettre_erreur_new ( scan_instance, "PID : input unknown. Select one R." );
       return(NULL);
     }
    if ( input->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : input must be R." );
       return(NULL);
     }

    struct ALIAS *consigne = Get_option_alias ( options, T_CONSIGNE );
    if (!consigne)
     { Emettre_erreur_new ( scan_instance, "PID : consigne unknown. Select one R." );
       return(NULL);
     }
    if ( consigne->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : consigne must be R." );
       return(NULL);;
     }

    struct ALIAS *kp = Get_option_alias ( options, T_KP );
    if (!kp)
     { Emettre_erreur_new ( scan_instance, "PID : kp. Select one R." );
       return(NULL);
     }
    if ( kp->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kp must be R." );
       return(NULL);
     }

    struct ALIAS *ki = Get_option_alias ( options, T_KI );
    if (!ki)
     { Emettre_erreur_new ( scan_instance, "PID : ki. Select one R." );
       return(NULL);
     }
    if ( ki->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : ki must be R." );
       return(NULL);
     }

    struct ALIAS *kd = Get_option_alias ( options, T_KD );
    if (!kd)
     { Emettre_erreur_new ( scan_instance, "PID : kd. Select one R." );
       return(NULL);
     }
    if ( kd->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : kd must be R." );
       return(NULL);;
     }

    struct ALIAS *output_min = Get_option_alias ( options, T_MIN );
    if (!output_min)
     { Emettre_erreur_new ( scan_instance, "PID : output_min. Select one R." );
       return(NULL);
     }
    if ( output_min->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output_min must be R." );
       return(NULL);
     }

    struct ALIAS *output_max = Get_option_alias ( options, T_MAX );
    if (!output_max)
     { Emettre_erreur_new ( scan_instance, "PID : output_max. Select one R." );
       return(NULL);
     }
    if ( output_max->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output_max must be R." );
       return(NULL);
     }

    struct ALIAS *output = Get_option_alias ( options, T_OUTPUT );
    if (!output)
     { Emettre_erreur_new ( scan_instance, "PID : output unknown. Select one R." );
       return(NULL);
     }
    if ( output->classe != T_REGISTRE )
     { Emettre_erreur_new ( scan_instance, "PID : output must be R." );
       return(NULL);
     }
    struct ACTION *action = New_action();
    gint taille=1024;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, " Dls_PID ( vars, _%s_%s, _%s_%s, _%s_%s, _%s_%s, _%s_%s, _%s_%s, _%s_%s, _%s_%s );",
                input->tech_id, input->acronyme,
                consigne->tech_id, consigne->acronyme,
                kp->tech_id, kp->acronyme,
                ki->tech_id, ki->acronyme,
                kd->tech_id, kd->acronyme,
                output_min->tech_id, output_min->acronyme,
                output_max->tech_id, output_max->acronyme,
                output->tech_id, output->acronyme );
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
 void Del_actions( struct ACTION *actions )
  { if (!actions) return;
    if (actions->alors) g_free(actions->alors);
    if (actions->sinon) g_free(actions->sinon);
    g_free(actions);
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

    if ( groupe==0 )                                              /* Un message groupé ne s'éteint jamais sur condition FALSE */
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
     { GSList *liste = Dls_scanner->Alias;                     /* Parsing de tous les alias de type monostable du meme groupe */
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
 struct ACTION *New_action_visuel( void *scan_instance, struct ALIAS *alias, GList *local_options )
  { struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *plugin_tech_id = Json_get_string ( Dls_scanner->PluginNode, "tech_id" );
    if (strcasecmp ( alias->tech_id, plugin_tech_id ))
     { Emettre_erreur_new ( scan_instance, "Setting VISUEL '%s:%s' out of plugin '%s' is forbidden",
                            alias->tech_id, alias->acronyme, plugin_tech_id );
       return(NULL);
     }

    gchar *forme = Get_option_chaine ( alias->options, T_FORME, NULL );           /* Récupère la forme depuis les definitions */
    if ( !forme )                                                          /* Normalement impossible car testé dans New_alias */
     { Emettre_erreur_new ( scan_instance, "'%s:%s': forme is unknown", alias->tech_id, alias->acronyme );
       return(NULL);
     }

    struct ACTION *action = New_action();
    gint taille = 1024;
    action->alors = New_chaine( taille );
    gchar chaine[256];

    gchar *mode = Get_option_chaine ( local_options, T_MODE, NULL );
    if ( mode )
     { if (! Dls_check_mode_VISUEL ( Dls_scanner, forme, mode ) )
        { Emettre_erreur_new ( scan_instance, "'%s:%s': mode '%s' is not known for forme '%s'", alias->tech_id, alias->acronyme, mode, forme ); }
       else
        { g_snprintf( chaine, sizeof(chaine),
                      "  Dls_data_VISUEL_set_mode ( vars, _%s_%s, \"%s\" );\n", alias->tech_id, alias->acronyme, mode );
          g_strlcat ( action->alors, chaine, taille );
        }
     }

    gchar *color = Get_option_chaine ( local_options, T_COLOR, NULL );
    if (color)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_color ( vars, _%s_%s, \"%s\" );\n", alias->tech_id, alias->acronyme, color );
       g_strlcat ( action->alors, chaine, taille );
     }

    gchar *libelle = Get_option_chaine ( local_options, T_LIBELLE, NULL );
    if (libelle)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_libelle ( vars, _%s_%s, \"%s\" );\n", alias->tech_id, alias->acronyme, libelle );
       g_strlcat ( action->alors, chaine, taille );
     }

    gchar *badge = Get_option_chaine ( local_options, T_BADGE, NULL );
    if (badge)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_badge ( vars, _%s_%s, \"%s\" );\n", alias->tech_id, alias->acronyme, badge );
       g_strlcat ( action->alors, chaine, taille );
     }

    struct ALIAS *input = Get_option_alias  ( alias->options, T_INPUT );
    gint   cligno       = Get_option_entier ( local_options, CLIGNO, 0 );
    gint   noshow       = Get_option_entier ( local_options, T_NOSHOW, 0 );
    gint   disable      = Get_option_entier ( local_options, T_DISABLE, 0 );

    if (!input)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set( vars, _%s_%s, 0.0, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_ANALOG_INPUT)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_AI( vars, _%s_%s, _%s_%s, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_CPT_IMP)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_CI( vars, _%s_%s, _%s_%s, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_CPT_H)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_CH( vars, _%s_%s, _%s_%s, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_REGISTRE)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_REGISTRE( vars, _%s_%s, _%s_%s, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_WATCHDOG)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_WATCHDOG( vars, _%s_%s, _%s_%s, %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else if (input->classe == T_TEMPO)
     { g_snprintf( chaine, sizeof(chaine),
                   "  Dls_data_VISUEL_set_for_TEMPO( vars, _%s_%s, _%s_%s,  %d, %d, %d );\n",
                   alias->tech_id, alias->acronyme, input->tech_id, input->acronyme, cligno, noshow, disable );
       g_strlcat ( action->alors, chaine, taille );
     }
    else Emettre_erreur_new ( scan_instance, "'%s:%s' is not allowed in 'input'", input->tech_id, input->acronyme );

    return(action);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bus( void *scan_instance,struct ALIAS *alias, GList *all_options )
  { struct ACTION *result;
    gint taille;

    struct DLS_TRAD *Dls_scanner = DlsScanner_get_extra ( scan_instance );
    gchar *target_tech_id = Get_option_chaine ( all_options, T_TECH_ID, Json_get_string ( Dls_scanner->PluginNode, "tech_id" ) );
    gchar *commande = Get_option_chaine ( all_options, T_COMMANDE, "" );
    g_strcanon ( commande, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_' ", ' ' );
    result = New_action();
    taille = 256+strlen(commande);
    result->alors = New_chaine( taille );
    g_snprintf( result->alors, taille, "  Dls_data_set_bus ( vars, \"%s\", \"%s\" );\n", target_tech_id, commande );
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
     { GSList *liste = Dls_scanner->Alias;                       /* Parsing de tous les alias de type bistable du meme groupe */
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

    g_snprintf( complement, sizeof(complement),
                "   Dls_data_set_BI ( vars, _%s_%s, %s );\n",
                alias->tech_id, alias->acronyme, (barre ? "FALSE" : "TRUE") );
    g_strlcat ( action->alors, complement, taille_alors );

    return(action);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
