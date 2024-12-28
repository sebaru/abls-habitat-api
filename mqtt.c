/******************************************************************************************************************************/
/* Watchdogd/mqtt.c        Fonctions communes de gestion des requetes MQTT                                                    */
/* Projet Abls-Habitat version 4.2   Gestion d'habitat                                                    12.07.2024 18:00:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt.c
 * This file is part of Watchdog
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
 static void MQTT_API_on_log_CB( struct mosquitto *mosq, void *obj, int level, const char *message )
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
 static void MQTT_API_on_message_CB ( struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg )
  { gchar **tokens = g_strsplit ( msg->topic, "/", 5 );
    if (!tokens) return;
    if (!tokens[0]) goto end; /* Normalement le domain_uuid  */
    if (!tokens[1]) goto end; /* Normalement le tag/topic  */

    struct DOMAIN *domain = DOMAIN_tree_get ( tokens[0] );
    if (!domain)
     { Info_new( __func__, LOG_ERR, NULL, "MQTT Message from unknown domain. Dropping for topic %s !", msg->topic );
       goto end;
     }

    if (!msg->payload)
     { Info_new( __func__, LOG_ERR, domain, "MQTT Message with no payload on topic %s", msg->topic );
       goto end;
     }

    JsonNode *request = Json_get_from_string ( msg->payload );
    if (!request)
     { Info_new( __func__, LOG_WARNING, domain, "MQTT Message Dropped (not JSON) for topic %s !", msg->topic );
       goto end;
     }

    Info_new( __func__, LOG_DEBUG, domain, "Received %s: %s", tokens[1], msg->payload );

    gchar *tag = tokens[1];
         if (!strcasecmp ( tag, "DLS_VISUEL"     ) ) { VISUEL_Handle_one        ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_HISTO"      ) ) { HISTO_Handle_one         ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_ARCHIVE"    ) ) { ARCHIVE_Handle_one       ( domain, request ); }
    else if (!strcasecmp ( tag, "DLS_REPORT"     ) )
     { if (! (tokens[2] && tokens[3] && tokens[4]) )
        { Info_new( __func__, LOG_ERR, domain, "TAG %s: no classe/tech_id/acronyme found, dropping", tag ); }
       else
        { Json_node_add_string ( request, "tech_id",  tokens[3] );
          Json_node_add_string ( request, "acronyme", tokens[4] );
               if (!strcasecmp ( tokens[2], "AI" ) ) Mnemo_sauver_un_AI ( domain, request );
          else if (!strcasecmp ( tokens[2], "AO" ) ) Mnemo_sauver_un_AO ( domain, request );
          else if (!strcasecmp ( tokens[2], "BI" ) ) Mnemo_sauver_un_BI ( domain, request );
          else if (!strcasecmp ( tokens[2], "CI" ) ) Mnemo_sauver_un_CI ( domain, request );
          else if (!strcasecmp ( tokens[2], "CH" ) ) Mnemo_sauver_un_CH ( domain, request );
          else Info_new( __func__, LOG_ERR, domain, "TAG %s: classe %s not found, dropping", tag, tokens[2] );
        }
     }
    else if (!strcasecmp ( tag, "HEARTBEAT"     ) ) { HEARTBEAT_Handle_one     ( domain, request ); }
    json_node_unref ( request );
end:
    g_strfreev( tokens );                                                                      /* Libération des tokens topic */
  }
/******************************************************************************************************************************/
/* Mqtt_Send_to_browsers: Envoie un message mqtt aux browsers d'un domain                                                     */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_to_browsers ( struct DOMAIN *domain, gchar *dest, gchar *tag, JsonNode *node )
  { if (! (domain && Global.MQTT_session && dest && tag) ) return;
    gchar topic[512];
    g_snprintf ( topic, sizeof(topic), "%s/browsers/%s/%s", Json_get_string ( domain->config, "domain_uuid" ), dest, tag );

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
/* Mqtt_Allow_pattern_for: souscrit à un pattern pour le domain et suffixe en parametre                                       */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Mqtt_Allow_pattern_for ( struct DOMAIN *domain, gchar *suffixe, gchar *topic )
  { gchar commande[1024];
    gint retour;
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"AddRoleACL\", "
                 "    \"rolename\": \"%s-%s\", "
                 "    \"acltype\": \"subscribePattern\", \"topic\": \"%s/%s\", \"priority\": -1, \"allow\": true "
                 "  } "
                 "  ] "
                 "}", domain_uuid, suffixe, domain_uuid, topic );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add %s-%s subscription for %s/%s failed, error %s",
                 domain_uuid, suffixe, domain_uuid, topic, mosquitto_strerror(retour) );
     }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"AddRoleACL\", "
                 "    \"rolename\": \"%s-%s\", "
                 "    \"acltype\": \"publishClientSend\", \"topic\": \"%s/%s\", \"priority\": -1, \"allow\": true "
                 "  } "
                 "  ] "
                 "}", domain_uuid, suffixe, domain_uuid, topic );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add %s-%s publishClientSend for %s/%s failed, error %s",
                 domain_uuid, suffixe, domain_uuid, topic, mosquitto_strerror(retour) );
     }

    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"AddRoleACL\", "
                 "    \"rolename\": \"%s-%s\", "
                 "    \"acltype\": \"publishClientReceive\", \"topic\": \"%s/%s\", \"priority\": -1, \"allow\": true "
                 "  } "
                 "  ] "
                 "}", domain_uuid, suffixe, domain_uuid, topic );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Add %s-%s publishClientReceive for %s/%s failed, error %s",
                 domain_uuid, suffixe, domain_uuid, topic, mosquitto_strerror(retour) );
     }
  }
/******************************************************************************************************************************/
/* Mqtt_Send_to_domain: Envoie un message mqtt a un domain                                                                    */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Allow_one_domain ( struct DOMAIN *domain )
  { gchar commande[1024];
    gint retour;
    if (! (Global.MQTT_session && domain) ) return;
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

/*------------------------------------------------------- Create Role --------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"createRole\", "
                 "    \"rolename\": \"%s-agents\", "
                 "    \"textname\": \"Role des agents du domaine %s\", "
                 "    \"textdescription\": \"Accès des agents du domaine %s\" "
                 "  } "
                 "  ] "
                 "}", domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Agent Role failed, error %s", mosquitto_strerror(retour) ); }

    Mqtt_Allow_pattern_for ( domain, "agents", "#" );
/*------------------------------------------------------- Create Client ------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"createClient\", "
                 "    \"username\": \"%s-agent\", "
                 "    \"password\": \"%s\", "
                 "    \"textname\": \"Agents dans le domaine %s\", "
                 "    \"textdescription\": \"Agents du domaine %s\", "
                 "    \"roles\": [ { \"rolename\": \"%s-agents\", \"priority\": -1 } ] "
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
                 "    \"rolename\": \"%s-browsers\", "
                 "    \"textname\": \"Role des browsers du domaine %s\", "
                 "    \"textdescription\": \"Accès des browsers du domaine %s\" "
                 "  } "
                 "  ] "
                 "}", domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Browsers Role failed, error %s", mosquitto_strerror(retour) ); }

    Mqtt_Allow_pattern_for ( domain, "browsers", "browsers/#" );
/*------------------------------------------------------- Create Browsers  ------------------------------------------------------*/
    g_snprintf ( commande, sizeof(commande),
                 "{ \"commands\":[ "
                 "  { "
                 "    \"command\": \"createClient\", "
                 "    \"username\": \"%s-browser\", "
                 "    \"password\": \"%s\", "
                 "    \"textname\": \"Browsers dans le domaine %s\", "
                 "    \"textdescription\": \"Browsers du domaine %s\", "
                 "    \"roles\": [ { \"rolename\": \"%s-browsers\", \"priority\": -1 } ] "
                 "  } "
                 "  ] "
                 "}", domain_uuid, Json_get_string ( domain->config, "browser_password" ), domain_uuid, domain_uuid, domain_uuid );

    retour = mosquitto_publish( Global.MQTT_session, NULL, "$CONTROL/dynamic-security/v1", strlen(commande), commande, 2, FALSE );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "MQTT Create Browsers domain failed, error %s", mosquitto_strerror(retour) ); }
  }
/******************************************************************************************************************************/
/* MQTT_Allow_for_domain_by_tree: Lance l'activation du MQTT pattern par tree                                                 */
/* Entrée: le gtree                                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean MQTT_Allow_one_domain_by_tree ( gpointer key, gpointer value, gpointer data )
  { if(!strcasecmp ( key, "master" )) return(FALSE);                                    /* Pas d'archive sur le domain master */
    struct DOMAIN *domain = value;
    MQTT_Allow_one_domain ( domain );
    return(FALSE);
  }

/******************************************************************************************************************************/
/* MQTT_local_on_connect_CB: appelé par la librairie quand le broker est connecté                                             */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_API_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, LOG_NOTICE, NULL, "Connected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
  }
/******************************************************************************************************************************/
/* MQTT_local_on_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                        */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_API_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, LOG_NOTICE, NULL, "Disconnected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
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

    mosquitto_log_callback_set        ( Global.MQTT_session, MQTT_API_on_log_CB );
    mosquitto_message_callback_set    ( Global.MQTT_session, MQTT_API_on_message_CB );
    mosquitto_connect_callback_set    ( Global.MQTT_session, MQTT_API_on_connect_CB );
    mosquitto_disconnect_callback_set ( Global.MQTT_session, MQTT_API_on_disconnect_CB );
    mosquitto_reconnect_delay_set     ( Global.MQTT_session, 10, 60, TRUE );

    if (Json_get_bool ( Global.config, "mqtt_over_ssl" ) )
     { mosquitto_tls_set( Global.MQTT_session, NULL, "/etc/ssl/certs", NULL, NULL, NULL ); }

    gchar *target = Json_get_string ( Global.config, "mqtt_hostname" );
    gint  port    = Json_get_int    ( Global.config, "mqtt_port" );
    mosquitto_username_pw_set( Global.MQTT_session, "api", Json_get_string ( Global.config, "mqtt_password" ) );
    retour = mosquitto_connect( Global.MQTT_session, target, port, 60 );
    if ( retour != MOSQ_ERR_SUCCESS )
        { Info_new( __func__, LOG_ERR, NULL, "MQTT Connection to '%s:%d' error: %s", target, port, mosquitto_strerror(retour) );
          return(FALSE);
        }
    Info_new( __func__, LOG_INFO, NULL, "MQTT starting connection to '%s:%d'.", target, port );

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
