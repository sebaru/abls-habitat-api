/******************************************************************************************************************************/
/* TraductionDLS/options.c          Gestion des options DLS                                                                   */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                14.04.2025 05:05:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * options.c
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
/* Liberer_options: Liberation de toutes les zones de mémoire précédemment allouées                                           */
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
 struct ALIAS *Get_option_alias( GList *liste_options, gint token )
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
 gchar *Get_option_chaine( GList *liste_options, gint token, gchar *defaut )
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
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 gdouble Get_option_double( GList *liste_options, gint token, gdouble defaut )
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
/*----------------------------------------------------------------------------------------------------------------------------*/
