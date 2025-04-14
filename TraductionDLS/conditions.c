/******************************************************************************************************************************/
/* TraductionDLS/conditions.c          Grdtion des conditions du langage DLS                                                  */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                14.04.2025 05:27:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * conditions.c
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
 void Del_condition( struct CONDITION *condition )
  { if (!condition) return;
    if (condition->chaine) g_free(condition->chaine);
    g_free(condition);
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
/*----------------------------------------------------------------------------------------------------------------------------*/
