/******************************************************************************************************************************/
/* Include/Dls.h                  Définitions des constantes programme DLS                                                    */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                14.07.2022 21:43:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls.h
 * This file is part of Habitat
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

 #ifndef _DLS_H_
  #define _DLS_H_

 enum
  { MNEMO_BISTABLE,                                                                   /* Definitions des types de mnemoniques */
    MNEMO_MONOSTABLE,
    MNEMO_TEMPO,
    MNEMO_ENTREE_TOR,
    MNEMO_SORTIE_TOR,
    MNEMO_ENTREE_ANA,
    MNEMO_SORTIE_ANA,
    MNEMO_VISUEL,
    MNEMO_CPTH,
    MNEMO_CPT_IMP,
    MNEMO_REGISTRE,
    MNEMO_HORLOGE,
    MNEMO_MSG,
    MNEMO_BUS,
    MNEMO_DIGITAL_OUTPUT,
    MNEMO_WATCHDOG,
    NBR_TYPE_MNEMO
  };

 enum
  { ARCHIVE_NONE,
    ARCHIVE_5_SEC,
    ARCHIVE_1_MIN,
    ARCHIVE_1_HEURE,
    ARCHIVE_1_JOUR
  };

 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_ATTENTE,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gint Traduire_DLS( gchar *tech_id );                                                                 /* Dans Interp.c */
 extern gboolean Mnemo_auto_create_BI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe );
 extern gboolean Mnemo_auto_create_MONO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src,
                                                    gchar *unite_src, gint archivage )  ;
 extern gboolean Mnemo_auto_create_AO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_HORLOGE ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_TEMPO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_REGISTRE ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_CI ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src, gdouble multi );
 extern gboolean Mnemo_auto_create_CH ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_MSG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint typologie, gint groupe );
 extern gboolean Mnemo_auto_create_VISUEL ( struct DOMAIN *domain, JsonNode *plugin, gchar *acronyme, gchar *libelle_src,
                                            gchar *forme_src, gchar *mode_src, gchar *couleur_src );
 extern gboolean Synoptique_auto_create_MOTIF ( struct DOMAIN *domain, JsonNode *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src );


 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
