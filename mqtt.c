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
/* MSRV_on_mqtt_message_CB: Appelé lorsque l'on recoit un message MQTT                                                        */
/* Entrée: les parametres MQTT                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_mqtt_message_CB ( struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg )
  {
   /* Recher che domaine */
   /* msg->topic*/
    struct DOMAIN *domain = DOMAIN_tree_get ( "test"/*domain_uuid*/ );
    if (!domain)
     { Info_new( __func__, LOG_ERR, NULL, "MQTT Message from unkonwn domain. Dropping !" );
       return;
     }

   JsonNode *request = Json_get_from_string ( msg->payload );
    if (!request)
     { Info_new( __func__, LOG_WARNING, domain, "MQTT Message Dropped (not JSON) !" );
       return;
     }
/*    Json_node_add_string ( request, "topic", msg->topic );

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Partage->com_msrv.MQTT_messages = g_slist_append ( Partage->com_msrv.MQTT_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );*/
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
 void MQTT_Send_to_domain ( struct DOMAIN *domain, gchar *topic, gchar *tag, JsonNode *node )
  { gboolean free_node=FALSE;
    if (! (Global.MQTT_session && topic && tag) ) return;
    if (!node) { node = Json_node_create(); free_node = TRUE; }
    Json_node_add_string ( node, "tag", tag );
    gchar *buffer = Json_node_to_string ( node );
    if (domain)
     { gchar topic_full[512];
       g_snprintf ( topic_full, sizeof(topic_full), "%s/%s", Json_get_string ( domain->config, "domain_uuid" ), topic );
       mosquitto_publish( Global.MQTT_session, NULL, topic_full, strlen(buffer), buffer, 0, FALSE );
     }
    else mosquitto_publish( Global.MQTT_session, NULL, topic, strlen(buffer), buffer, 0, FALSE );
    g_free(buffer);
    if (free_node) json_node_unref(node);
  }
/******************************************************************************************************************************/
/* MQTT_Subscribe: souscrit à un topic                                                                                        */
/* Entrée: la structure MQTT, le topic                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Subscribe ( struct DOMAIN *domain, gchar *topic )
  { if ( mosquitto_subscribe( Global.MQTT_session, NULL, topic, 0 ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, domain, "Subscribe to topic '%s' FAILED", topic ); }
    else
     { Info_new( __func__, LOG_INFO, domain, "Subscribe to topic '%s' OK", topic ); }
  }
/******************************************************************************************************************************/
/* MQTT_Start: Demarre l'ecoute MQTT                                                                                          */
/* Entrée: néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean MQTT_Start ( void )
  { mosquitto_lib_init();
    Global.MQTT_session = mosquitto_new( "api", FALSE, NULL );
    if (!Global.MQTT_session)
     { Info_new( __func__, LOG_ERR, NULL, "MQTT session error." ); return(FALSE); }
    gchar *target = Json_get_string ( Global.config, "mqtt_hostname" );
    if ( mosquitto_connect( Global.MQTT_session, target, 1883, 60 ) != MOSQ_ERR_SUCCESS )
        { Info_new( __func__, LOG_ERR, NULL, "MQTT Connection to '%s' error.", target );
          return(FALSE);
        }
    mosquitto_message_callback_set( Global.MQTT_session, MQTT_on_mqtt_message_CB );
    MQTT_Subscribe ( NULL, "api/#" );
    if ( mosquitto_loop_start( Global.MQTT_session ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, LOG_ERR, NULL, "MQTT loop not started." ); return(FALSE); }
    Info_new( __func__, LOG_INFO, NULL, "MQTT Connection to '%s' successfull.", target );
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
