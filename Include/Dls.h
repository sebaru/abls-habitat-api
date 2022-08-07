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

 enum                                                                                  /* Code retour de la compilation D.L.S */
  { DLS_COMPIL_NEVER_COMPILED_YET,
    DLS_COMPIL_EXPORT_DB_FAILED,
    DLS_COMPIL_ERROR_LOAD_SOURCE,
    DLS_COMPIL_ERROR_LOAD_LOG,
    DLS_COMPIL_SYNTAX_ERROR,
    DLS_COMPIL_ERROR_FORK_GCC,
    DLS_COMPIL_OK,
    DLS_COMPIL_OK_WITH_WARNINGS,
    NBR_DLS_COMPIL_STATUS
  };

 enum { TRAD_DLS_OK,                                                                   /* Retour de la fonction Traduire DLS. */
        TRAD_DLS_WARNING,
        TRAD_DLS_SYNTAX_ERROR,
        TRAD_DLS_ERROR_NO_FILE
      };

 #define NBR_CARAC_TECHID     32
 #define NBR_CARAC_ACRONYME   64
 #define NBR_CARAC_UNITE      32

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

 enum
  { MESSAGE_SMS_NONE,
    MESSAGE_SMS_YES,
    MESSAGE_SMS_GSM_ONLY,
    MESSAGE_SMS_OVH_ONLY,
    NBR_TYPE_MESSAGE_SMS
  };
  
 enum                                                                                  /* différent statut des temporisations */
  { DLS_TEMPO_NOT_COUNTING,                                                                 /* La tempo ne compte pas du tout */
    DLS_TEMPO_WAIT_FOR_DELAI_ON,                                       /* La tempo compte, en attendant le delai de mise à un */
    DLS_TEMPO_WAIT_FOR_MIN_ON,                                         /* Delai de MAU dépassé, en attente du creneau minimum */
    DLS_TEMPO_WAIT_FOR_MAX_ON,                                      /* Creneau minimum atteint, en attente du creneau maximum */
    DLS_TEMPO_WAIT_FOR_DELAI_OFF,                                /* Creneau max atteint, en attente du delai de remise a zero */
    DLS_TEMPO_WAIT_FOR_COND_OFF                                            /* Attend que la condition soit tombée avant reset */
  };

 struct DLS_TEMPO                                                                           /* Définition d'une temporisation */
  { gchar   acronyme[NBR_CARAC_ACRONYME];
    gchar   tech_id[NBR_CARAC_TECHID];
    gboolean init;                                   /* True si les données delai_on/off min_on/off ont bien été positionnées */
    guint status;                                                                               /* Statut de la temporisation */
    guint date_on;                                                              /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                                            /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
    guint delai_on;                                                     /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                                                  /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;                            /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;                            /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint random;                                         /* Est-ce une tempo random ? si oui, est la dynamique max du random */
  };

 struct DLS_AI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gchar   unite[32];                                                                                        /* Km, h, ° ... */
    gdouble valeur;
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   in_range;
    guint   archivage;
   };

 struct DLS_AO
  { gchar   acronyme[NBR_CARAC_ACRONYME];
    gchar   tech_id[NBR_CARAC_TECHID];
    gdouble min;
    gdouble max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
    gchar   unite[NBR_CARAC_UNITE];                                                                           /* Km, h, ° ... */
    gdouble valeur;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct SORTIE_TOR                                                                             /* Définition d'une sortie TOR */
  { gchar etat;                                                                                   /* Etat de la sortie 0 ou 1 */
    gint last_change;                                                                    /* Date du dernier changement d'etat */
  };

 struct DLS_WATCHDOG
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    top;
  };

 struct DLS_MONO
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_BI
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    groupe; /* Groupe 'radio' */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DI
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DO
  { gchar   tech_id[32];
    gchar   acronyme[64];
    gchar   libelle[128];                                                                                     /* Km, h, ° ... */
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_CI
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    valeur;
    gint    val_en_cours1;                                                    /* valeur en cours pour le calcul via les ratio */
    gdouble ratio;
    gdouble multi;
    guint   last_update;
    gint    imp_par_minute;
    gint    valeurs[60];                                                                              /* 60 dernieres valeurs */
    gchar   unite[NBR_CARAC_UNITE];
    gboolean etat;
    gint    archivage;
    guint   last_arch;
  };

 struct DLS_CH
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    guint valeur;
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
    guint old_top;                                                                         /* Date de debut du comptage du CH */
    gboolean etat;
  };

 struct DLS_VISUEL
  { gchar    tech_id[NBR_CARAC_TECHID];
    gchar    acronyme[NBR_CARAC_ACRONYME];
    gchar    mode[32];
    gchar    color[16];
    gboolean cligno;
    gint     last_change;
    gint     changes;
    gchar    libelle[128]; /* libelle issu du plugin DLS */
  };

 struct DLS_MESSAGES
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;
    gboolean etat_update;
    gint groupe;
    gint last_change;
    gint last_on;
    gint changes;
  };

 struct DLS_MESSAGES_EVENT
  { struct DLS_MESSAGES *msg;
    gboolean etat;
  };

 struct DLS_REGISTRE
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gdouble valeur;
    gchar   unite[NBR_CARAC_UNITE];
    gint    archivage;
    guint   last_arch;                                                   /* Date de dernier enregistrement en base de données */
    gdouble pid_somme_erreurs;                                                                                /* Calcul PID KI*/
    gdouble pid_prev_erreur;                                                                                 /* Calcul PID KD */
  };

 struct DLS_SYN
  { gint syn_id;
    gboolean bit_comm;
    gboolean bit_defaut;
    gboolean bit_defaut_fixe;
    gboolean bit_alarme;
    gboolean bit_alarme_fixe;
    gboolean bit_veille_partielle;
    gboolean bit_veille_totale;
    gboolean bit_alerte;
    gboolean bit_alerte_fixe;
    gboolean bit_alerte_fugitive;
    gboolean bit_derangement;
    gboolean bit_derangement_fixe;
    gboolean bit_danger;
    gboolean bit_danger_fixe;
    GSList *Dls_plugins;                                                     /* Liste des plugins D.L.S associé au synoptique */
    GSList *Dls_sub_syns;                                                    /* Liste des plugins D.L.S associé au synoptique */
  };

 struct COM_DLS                                                                      /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    pthread_mutex_t synchro_traduction;                  /* Mutex pour interdire les traductions simultanées de plugins D.L.S */
    pthread_mutex_t synchro_data;                                      /* Mutex pour les acces concurrents à l'arbre des data */
    GSList *Dls_plugins;                                                                             /* Liste d'execution DLS */
    struct DLS_SYN *Dls_syns;                                                              /* Arbre de calcule des etats */
    GSList *Set_Dls_DI_Edge_up;                                                 /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_DI_Edge_down;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_Data;                                                       /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_up;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_down;                                           /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_up;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_down;                                           /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_Data;                                               /* liste des Mxxx a désactiver à la fin du tour prg */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_reload_with_recompil;                       /* TRUE si le thread doit rebooter en recompilant les modules */
    guint temps_sched;                                          /* Delai d'attente DLS pour assurer 100 tours max par seconde */
    gboolean Top_check_horaire;                                                    /* True le controle horaire est réalisable */
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gint Traduire_DLS( gchar *tech_id );                                                                 /* Dans Interp.c */
 extern gboolean Mnemo_auto_create_BI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe );
 extern gboolean Mnemo_auto_create_MONO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_AO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_HORLOGE ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_TEMPO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_REGISTRE ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_CI ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src, gdouble multi );
 extern gboolean Mnemo_auto_create_CH ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
