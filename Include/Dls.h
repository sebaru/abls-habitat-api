/******************************************************************************************************************************/
/* Include/Dls.h                  Définitions des constantes programme DLS                                                    */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                14.07.2022 21:43:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls.h
 * This file is part of Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

 #define ARCHIVE_NONE           0
 #define ARCHIVE_5_SEC          50
 #define ARCHIVE_1_MIN          600
 #define ARCHIVE_1_HEURE        36000
 #define ARCHIVE_1_JOUR         864000

 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_NOTIF,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gint Traduire_DLS( gchar *tech_id );                                                                 /* Dans Interp.c */
 extern gboolean Mnemo_auto_create_BI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe );
 extern gboolean Mnemo_auto_create_MONO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );
 extern gboolean Mnemo_auto_create_AI_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src,
                                                    gchar *unite_src, gint archivage );
 extern gboolean Mnemo_auto_create_AO_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );
 extern gboolean Mnemo_auto_create_AO_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src,
                                                    gchar *unite_src, gint archivage );
 extern gboolean Mnemo_auto_create_DI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );
 extern gboolean Mnemo_auto_create_DI_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DO_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );
 extern gboolean Mnemo_auto_create_DO_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gboolean mono );
 extern gboolean Mnemo_auto_create_HORLOGE_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_HORLOGE_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_TEMPO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_REGISTRE ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_CI ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src, gdouble multi );
 extern gboolean Mnemo_auto_create_CH ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_MSG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint typologie, gint groupe );
 extern gboolean Mnemo_auto_create_VISUEL ( struct DOMAIN *domain, JsonNode *plugin, gchar *acronyme, gchar *libelle_src,
                                            gchar *forme_src, gchar *mode_src, gchar *couleur_src,
                                            gdouble min, gdouble max, gdouble seuil_ntb, gdouble seuil_nb, gdouble seuil_nh, gdouble seuil_nth,
                                            gint decimal, gchar *input_tech_id_src, gchar *input_acronyme_src  );
 extern gboolean Synoptique_auto_create_MOTIF ( struct DOMAIN *domain, JsonNode *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src, gint place );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
