/******************************************************************************************************************************/
/* Watchdogd/mqtt.c        Fonctions communes de gestion des requetes MQTT                                                    */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    12.07.2024 18:00:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt.c
 * This file is part of Watchdog
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

/*************************************************** Prototypes de fonctions ***************************************************/
 #include <mosquitto.h>
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* HEARTBEAT_Handle_one: Traite un heartbeat recu par mqtt                                                                    */
/* Entrées: le jsonnode représentant la source                                                                                */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void HEARTBEAT_Handle_one ( struct DOMAIN *domain, JsonNode *source )
  { if (!source) return;
    if (Json_has_member ( source, "agent_uuid" ) )                                         /* Est-ce un agent qui nous bipe ? */
     { gchar *agent_uuid = Normaliser_chaine ( Json_get_string ( source, "agent_uuid" ) );
       DB_Write ( domain, "UPDATE agents SET heartbeat_time = NOW() WHERE agent_uuid='%s'", agent_uuid );
       g_free(agent_uuid);
     }
    else if (Json_has_member ( source, "thread_tech_id" ) )                               /* Est-ce un thread qui nous bipe ? */
     { THREAD_HEARTBEAT_set ( domain, source ); }
  }
/******************************************************************************************************************************/
/* MQTT_on_log_CB: Affiche un log de la librairie MQTT                                                                        */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_log_CB( struct mosquitto *mosq, void *obj, int level, const char *message )
  { gint info_level;
    switch(level)
     { default:
       case MOSQ_LOG_INFO:    info_level = LOG_INFO;    break;
       case MOSQ_LOG_NOTICE:  info_level = LOG_NOTICE;  break;
       case MOSQ_LOG_WARNING: info_level = LOG_WARNING; break;
       case MOSQ_LOG_ERR:     info_level = LOG_ERR;     break;
       case MOSQ_LOG_DEBUG:   info_level = LOG_DEBUG;   break;
     }
    Info_new( __func__, info_level, NULL, "%s", message );
  }
/******************************************************************************************************************************/
/* MSRV_on_mqtt_message_CB: Appelé lorsque l'on recoit un message MQTT                                                        */
/* Entrée: les parametres MQTT                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_mqtt_message_CB ( struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg )
  { gchar **tokens = g_strsplit ( msg->topic, "/", 2 );
    if (!tokens) return;
    if (!tokens[0]) goto end; /* Normalement le domain_uuid  */
    if (!tokens[1]) goto end; /* Normalement le tag/topic  */

    struct DOMAIN *domain = DOMAIN_tree_get ( tokens[0] );
    if (!domain)
     { Info_new( __func__, LOG_ERR, NULL, "MQTT Message from unknown domain. Dropping !" );
       goto end;
     }

    JsonNode *request = Json_get_from_string ( msg->payload );
    if (!request)
     { Info_new( __func__, LOG_WARNING, domain, "MQTT Message Dropped (not JSON) !" );
       goto end;
     }

    Info_new( __func__, LOG_DEBUG, domain, "Received %s: %s", tokens[1], msg->payload );

    gchar *tag = tokens[1];
         if (!strcasecmp ( tag, "DLS_VISUEL"     ) ) { VISUEL_Handle_one        ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_HISTO"      ) ) { HISTO_Handle_one         ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_ABONNEMENT" ) ) { ABONNEMENT_Handle_one    ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_ARCHIVE"    ) ) { ARCHIVE_Handle_one       ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_REPORT"     ) ) { MNEMOS_REPORT_Handle_one ( domain, request ); }
    else if (!strcasecmp ( tag, "HEARTBEAT"      ) ) { HEARTBEAT_Handle_one     ( domain, request ); }
       /*    Json_node_add_string ( request, "topic", msg->topic );

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Partage->com_msrv.MQTT_messages = g_slist_append ( Partage->com_msrv.MQTT_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );*/
end:
    g_strfreev( tokens );                                                                      /* Libération des tokens topic */
  }
/******************************************************************************************************************************/
/* MSRV_Handle_MQTT_messages: Appelé lorsque l'on recoit un message MQTT                                                      */
/* Entrée: les parametres MQTT                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
/* static void MSRV_Handle_MQTT_messages( void )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    JsonNode *request = Partage->com_msrv.MQTT_messages->data;
    Partage->com_msrv.MQTT_messages = g_slist_remove ( Partage->com_msrv.MQTT_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
    gchar *tag = Json_get_string ( request, "tag" );
    if (!tag) goto end;

         if ( !strcmp ( tag, "SET_AI" ) )       Dls_data_set_AI_from_thread_ai ( request );
    else if ( !strcmp ( tag, "SET_DI" ) )       Dls_data_set_DI_from_thread_di ( request );
    else if ( !strcmp ( tag, "SET_WATCHDOG" ) ) Dls_data_set_WATCHDOG_from_thread_watchdog ( request );
    else if ( !strcmp ( tag, "SET_DI_PULSE" ) )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
        { Info_new( __func__, Config.log_bus, LOG_ERR, "SET_DI_PULSE: wrong parameters" ); }
       else { gchar *thread_tech_id = Json_get_string ( request, "thread_tech_id" );
              gchar *tech_id        = Json_get_string ( request, "tech_id" );
              gchar *acronyme       = Json_get_string ( request, "acronyme" );
              Info_new( __func__, Config.log_bus, LOG_INFO, "SET_DI_PULSE from '%s': '%s:%s'=1", thread_tech_id, tech_id, acronyme );
              struct DLS_DI *bit = Dls_data_lookup_DI ( tech_id, acronyme );
              Dls_data_set_DI_pulse ( NULL, bit );
            }
     }
end:
    Json_node_unref ( request );
  }*/
/******************************************************************************************************************************/
/* Mqtt_Send_to_domain: Envoie un message mqtt a un domain                                                                    */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_to_domain ( struct DOMAIN *domain, gchar *dest, gchar *tag, JsonNode *node )
  { if (! (domain && Global.MQTT_session && dest && tag) ) return;
    gchar topic[512];
    g_snprintf ( topic, sizeof(topic), "%s/%s/%s", Json_get_string ( domain->config, "domain_uuid" ), dest, tag );

    if (!node)
     { mosquitto_publish( Global.MQTT_session, NULL, topic, 0, NULL, Json_get_int ( Global.config, "mqtt_qos" ), FALSE );
       return;
     }

    gchar *buffer = Json_node_to_string ( node );
    if (buffer)
     { mosquitto_publish( Global.MQTT_session, NULL, topic, strlen(buffer), buffer, Json_get_int ( Global.config, "mqtt_qos" ), FALSE );
       g_free(buffer);
     }
  }
/******************************************************************************************************************************/
/* Mqtt_Send_to_domain: Envoie un message mqtt a un domain                                                                    */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Allow_for_domain ( struct DOMAIN *domain )
  { gchar commande[1024];
    gint retour;
    if (! (Global.MQTT_session && domain) ) return;
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

/*------------------------------------------------------- Create Role --------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"createRole\", "
			              "    \"rolename\": \"agents-%s\", "
			              "    \"textname\": \"Role des agents du domaine %s\", "
			              "    \"textdescription\": \"Accès des agents du domaine %s\" "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Agent Role failed, error %s", mosquitto_strerror(retour) ); }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"agents-%s\", "
				             "    \"acltype\": \"subscribePattern\", \"topic\": \"%s/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add Agent SubscribePattern failed, error %s", mosquitto_strerror(retour) ); }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"agents-%s\", "
				             "    \"acltype\": \"publishClientSend\", \"topic\": \"%s/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT add Agent publishClientSend failed, error %s", mosquitto_strerror(retour) ); }


    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"agents-%s\", "
				             "    \"acltype\": \"publishClientReceive\", \"topic\": \"%s/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT add Agent publishClientReceive failed, error %s", mosquitto_strerror(retour) ); }

/*------------------------------------------------------- Create Client ------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"createClient\", "
			              "    \"username\": \"agent-%s\", "
			              "    \"password\": \"%s\", "
			              "    \"textname\": \"Agents dans le domaine %s\", "
			              "    \"textdescription\": \"Agents du domaine %s\", "
                 "    \"roles\": [	{ \"rolename\": \"agents-%s\", \"priority\": -1 } ] "
		               "  } "
	                "  ] "
                 "}", domain_uuid, Json_get_string ( domain->config, "mqtt_password" ), domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Agents domain failed, error %s", mosquitto_strerror(retour) ); }
/*------------------------------------------------------- Create Browser Role ------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"createRole\", "
			              "    \"rolename\": \"browsers-%s\", "
			              "    \"textname\": \"Role des browsers du domaine %s\", "
			              "    \"textdescription\": \"Accès des browsers du domaine %s\" "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Browsers Role failed, error %s", mosquitto_strerror(retour) ); }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"browsers-%s\", "
				             "    \"acltype\": \"subscribePattern\", \"topic\": \"%s/DLS_VISUEL/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add Browsers SubscribePattern VISUEL failed, error %s", mosquitto_strerror(retour) ); }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"browsers-%s\", "
				             "    \"acltype\": \"subscribePattern\", \"topic\": \"%s/DLS_HISTO/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add Browsers SubscribePattern HISTO failed, error %s", mosquitto_strerror(retour) ); }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"AddRoleACL\", "
			              "    \"rolename\": \"browsers-%s\", "
				             "    \"acltype\": \"publishClientReceive\", \"topic\": \"%s/DLS_VISUEL/#\", \"priority\": -1, \"allow\": true "
		               "  } "
	                "  ] "
                 "}", domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT add Browsers publishClientReceive failed, error %s", mosquitto_strerror(retour) ); }

/*------------------------------------------------------- Create Browsers  ------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
		               "  { "
			              "    \"command\": \"createClient\", "
			              "    \"username\": \"browser-%s\", "
			              "    \"password\": \"%s\", "
			              "    \"textname\": \"Browsers dans le domaine %s\", "
			              "    \"textdescription\": \"Browsers du domaine %s\", "
                 "    \"roles\": [	{ \"rolename\": \"browsers-%s\", \"priority\": -1 } ] "
		               "  } "
	                "  ] "
                 "}", domain_uuid, Json_get_string ( domain->config, "mqtt_password" ), domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Browsers domain failed, error %s", mosquitto_strerror(retour) ); }
  }
/******************************************************************************************************************************/
/* MQTT_Start: Demarre l'ecoute MQTT                                                                                          */
/* Entrée: néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean MQTT_Start ( void )
  { gint retour;
    mosquitto_lib_init();
    Global.MQTT_session = mosquitto_new( "api", FALSE, NULL );
    if (!Global.MQTT_session) { Info_new( __func__, LOG_ERR, NULL, "MQTT session error." ); return(FALSE); }
    mosquitto_username_pw_set(	Global.MQTT_session, "api", Json_get_string ( Global.config, "mqtt_password" ) );

    if (Json_get_bool ( Global.config, "mqtt_over_ssl" ) )
     { mosquitto_tls_set( Global.MQTT_session, NULL, "/etc/ssl/certs", NULL, NULL, NULL ); }

    gchar *target = Json_get_string ( Global.config, "mqtt_hostname" );
    gint  port    = Json_get_int    ( Global.config, "mqtt_port" );
    retour = mosquitto_connect( Global.MQTT_session, target, port, 60 );
    if ( retour != MOSQ_ERR_SUCCESS )
        { Info_new( __func__, LOG_ERR, NULL, "MQTT Connection to '%s:%d' error: %s", target, port, mosquitto_strerror(retour) );
          return(FALSE);
        }
    Info_new( __func__, LOG_INFO, NULL, "MQTT starting connection to '%s:%d'.", target, port );

    mosquitto_log_callback_set    ( Global.MQTT_session, MQTT_on_log_CB );
    mosquitto_message_callback_set( Global.MQTT_session, MQTT_on_mqtt_message_CB );
    mosquitto_reconnect_delay_set ( Global.MQTT_session, 10, 60, TRUE );

    retour =  mosquitto_subscribe( Global.MQTT_session, NULL, "#", 1 );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, NULL, "Subscribe to topic '#' FAILED: %s", mosquitto_strerror(retour) ); }
    else
     { Info_new( __func__, LOG_INFO, NULL, "Subscribe to topic '#' OK" ); }

    retour = mosquitto_loop_start( Global.MQTT_session );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, NULL, "MQTT loop not started: %s", mosquitto_strerror(retour) );
       return(FALSE);
     }
    Info_new( __func__, LOG_INFO, NULL, "MQTT Connection to '%s:%d' successfull.", target, port );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* MQTT_Start: Arret de l'ecoute MQTT                                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Stop ( void )
  { mosquitto_disconnect( Global.MQTT_session );
    mosquitto_loop_stop( Global.MQTT_session, FALSE );
    mosquitto_destroy( Global.MQTT_session );
    mosquitto_lib_cleanup();
    Info_new( __func__, LOG_INFO, NULL, "MQTT Stopped" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
