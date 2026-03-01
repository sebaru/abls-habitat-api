/******************************************************************************************************************************/
/* json.c           Fonctions helper pour la manipulation des payload au format JSON                                          */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                                16.02.2022 09:37:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * json.c
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>

 #include "Http.h"

/******************************************************************************************************************************/
/* Json_create: Prepare un RootNode pour creer un nouveau buffer json                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 JsonNode *Json_node_create ( void )
  { JsonNode *RootNode;
    RootNode = json_node_alloc();
    json_node_take_object ( RootNode, json_object_new() );
    return(RootNode);
  }
/******************************************************************************************************************************/
/* Json_copy_member_into: Copie un membre d'un node vers un autre                                                             */
/* Entrée: Le source node, le nom du membre, le source destination                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_copy_member_into ( JsonNode *SrcNode, gchar *name, JsonNode *DestNode )
  { JsonObject *src_obj  = json_node_get_object(SrcNode);                                   /* Récupération de l'objet source */
    if (!src_obj) return;
    JsonObject *dest_obj = json_node_get_object(DestNode);                             /* Récupération de l'objet destination */
    if (!dest_obj) return;
    JsonNode *src_member_node = json_object_get_member( src_obj, name );        /* Récupération du membre name dans l'obj_src */
    if (!src_member_node) return;
    json_object_set_member ( dest_obj, name, json_node_copy(src_member_node) );                  /* Copie dans la destination */
  }
/******************************************************************************************************************************/
/* Json_to_log: Dump JsonNode to log                                                                                          */
/* Entrée: Le node json a dumper                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_to_log ( struct DOMAIN *domain, gchar *prefix, JsonNode *RootNode )
  { const char *name;
    JsonObjectIter iter;
    JsonNode *ObjectMemberNode;
    if (!prefix) prefix="";
    JsonObject *RootObject = json_node_get_object(RootNode);                                /* Récupération de l'objet source */
    json_object_iter_init(&iter, RootObject);
    while (json_object_iter_next(&iter, &name, &ObjectMemberNode))
     { JsonNodeType value_json_type = json_node_get_node_type ( ObjectMemberNode );
       switch (value_json_type)
        { default:
          case JSON_NODE_NULL:   break;
          case JSON_NODE_OBJECT: break;
          case JSON_NODE_ARRAY:  break;
          case JSON_NODE_VALUE:
           { GType valueType = json_node_get_value_type( ObjectMemberNode );
             if (valueType == G_TYPE_INT64)
              { Info_new ( __func__, LOG_INFO, domain, "%s: %s = %d", prefix, name, json_node_get_int(ObjectMemberNode) ); }
             else if (valueType == G_TYPE_DOUBLE)
              { Info_new ( __func__, LOG_INFO, domain, "%s: %s = %f", prefix, name, json_node_get_double(ObjectMemberNode) ); }
             else if (valueType == G_TYPE_BOOLEAN)
              { Info_new ( __func__, LOG_INFO, domain, "%s: %s = %s", prefix, name, ( json_node_get_boolean(ObjectMemberNode) ? "true" : "false") ); }
             else if (valueType == G_TYPE_STRING)
              { if (g_strrstr ( name, "password" ))
                 { Info_new ( __func__, LOG_INFO, domain, "%s: %s = ******", prefix, name ); }
                else
                 { Info_new ( __func__, LOG_INFO, domain, "%s: %s = %s", prefix, name, json_node_get_string(ObjectMemberNode) ); }
              }
           }
        }
     }
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_string ( JsonNode *RootNode, gchar *name, gchar *chaine )
  { JsonObject *object = json_node_get_object (RootNode);
    if (chaine) json_object_set_string_member ( object, name, chaine );
           else json_object_set_null_member   ( object, name );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_bool ( JsonNode *RootNode, gchar *name, gboolean valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_boolean_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_node_add_double: Ajoute un enregistrement name/double dans le RootNode                                                */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_double ( JsonNode *RootNode, gchar *name, gdouble valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_double_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_int ( JsonNode *RootNode, gchar *name, gint64 valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_int_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_node_add_null: Ajoute un enregistrement NULL dans le RootNode                                                         */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_null ( JsonNode *RootNode, gchar *name )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_null_member   ( object, name );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonArray *Json_node_add_array ( JsonNode *RootNode, gchar *name )
  { JsonObject *object = json_node_get_object (RootNode);
    JsonArray *tableau = json_array_new();
    json_object_set_array_member ( object, name, tableau );
    return(tableau);
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Json_node_add_objet ( JsonNode *RootNode, gchar *name )
  { JsonObject *RootObject = json_node_get_object (RootNode);
    JsonNode *new_node = json_node_alloc();
    json_node_set_object ( new_node, json_object_new() );
    json_object_set_member ( RootObject, name, new_node );
    return(new_node);
  }
/******************************************************************************************************************************/
/* Json_array_add_element: Ajoute un enregistrement dans le tableau                                                           */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_array_add_element ( JsonArray *array, JsonNode *element )
  { json_array_add_element ( array, element ); }
/******************************************************************************************************************************/
/* Json_node_foreach_array_element: Lance une fonction ne parametre sur chacun des elements d'un tableau                      */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_foreach_array_element ( JsonNode *RootNode, gchar *nom, JsonArrayForeach fonction, gpointer data )
  { json_array_foreach_element ( Json_get_array ( RootNode, nom ), fonction, data ); }
/******************************************************************************************************************************/
/* Json_node_to_string: transforme un JsonNode en string                                                                      */
/* Entrée: le JsonNode a convertir                                                                                            */
/* Sortie: un nouveau buffer                                                                                                  */
/******************************************************************************************************************************/
 gchar *Json_node_to_string ( JsonNode *RootNode )
  { return ( json_to_string ( RootNode, FALSE ) );
  }
/******************************************************************************************************************************/
/* Json_get_from_stirng: Recupere l'object de plus haut niveau dans une chaine JSON                                           */
/* Entrée: la chaine de caractere                                                                                             */
/* Sortie: l'objet                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Json_get_from_string ( gchar *chaine )
  { return(json_from_string ( chaine, NULL )); }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gchar *Json_get_string ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(NULL); }
    return(json_object_get_string_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gdouble Json_get_double ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(0.0); }
    return(json_object_get_double_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_get_bool ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(FALSE); }
    return(json_object_get_boolean_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_int: Recupere l'entier dont le nom est en parametre                                                               */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gint Json_get_int ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(0); }
    return(json_object_get_int_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonArray *Json_get_array ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(NULL); }
    return(json_object_get_array_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonObject *Json_get_object_as_object ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(NULL); }
    return(json_object_get_object_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonNode *Json_get_object_as_node ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(NULL); }
    return(json_object_get_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_int: Recupere l'entier dont le nom est en parametre                                                               */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_has_member ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!query)
     { Info_new ( __func__, LOG_ERR, NULL, "Query is null for '%s'", chaine );  return(FALSE); }
    if (!object)
     { Info_new ( __func__, LOG_ERR, NULL, "Object is null for '%s'", chaine );  return(FALSE); }
    if (!json_object_has_member ( object, chaine ))
     { Info_new ( __func__, LOG_DEBUG, NULL, "%s is missing", chaine ); return(FALSE); }
    if (json_object_get_null_member ( object, chaine ))
     { Info_new ( __func__, LOG_DEBUG, NULL, "%s is null", chaine ); return(FALSE); }
    return( TRUE );
  }
/******************************************************************************************************************************/
/* Json_read_from_file: Recupere un ficher et le lit au format Json                                                           */
/* Entrée: le nom de fichier                                                                                                  */
/* Sortie: le buffer JsonNode                                                                                                 */
/******************************************************************************************************************************/
 JsonNode *Json_read_from_file ( gchar *filename )
  { struct stat stat_buf;
    if (stat ( filename, &stat_buf)==-1) return(NULL);

    gchar *content = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!content) return(NULL);

    gint fd = open ( filename, O_RDONLY );
    if (!fd)
     { Info_new ( __func__, LOG_ERR, NULL, "Unable to open Config file %s: %s", filename, strerror(errno) );
       g_free(content);
       return(NULL);
     }

    if (read ( fd, content, stat_buf.st_size ) != stat_buf.st_size)
     { Info_new ( __func__, LOG_ERR, NULL, "Unable to read Config file %s: %s", filename, strerror(errno) );
       g_free(content);
       return(NULL);
     }
    close(fd);

    JsonNode *node = Json_get_from_string ( content );
    if (!node) Info_new ( __func__, LOG_ERR, NULL, "Unable to parse: Config file %s is not JSON", filename );

    g_free(content);
    return(node);
  }
/******************************************************************************************************************************/
/* Json_read_config: Recupere un ficher de config, rempli les manques avec l'environnement, ou applique une valeur par defaut */
/* Entrée: le nom de fichier, le JsonNode des défauts                                                                         */
/* Sortie: un nouveau buffer JsonNode qui reunit le meilleur des 3 mondes (env, file, default)                                */
/******************************************************************************************************************************/
 void Json_read_config ( gchar *filename, JsonNode *target )
  { const char *name;
    JsonObjectIter iter;
    JsonNode *ObjectMemberNode;
    JsonNode *from_file = Json_read_from_file ( filename );
    if (from_file)                                                              /* Copy des elements de from_file vers target */
     { Info_new ( __func__, LOG_NOTICE, NULL, "Reading config file '%s'", filename );
       JsonObject *fromFileObject = json_node_get_object(from_file);                        /* Récupération de l'objet source */
       json_object_iter_init(&iter, fromFileObject);
       while (json_object_iter_next(&iter, &name, &ObjectMemberNode))
        { Json_copy_member_into ( from_file, name, target ); }
       json_node_unref( from_file );
     } else Info_new ( __func__, LOG_ERR, NULL, "Unable to read file config '%s'", filename );

    gchar **env_vars = g_listenv();                                       /* Récupérer la liste des variables d'environnement */
    for (gchar **env = env_vars; *env != NULL; env++)                                       /* Parcourir toutes les variables */
     { gchar *prefixe = "ABLS_";
       if (g_str_has_prefix(*env, prefixe))                                   /* Vérifier si la variable commence par "ABLS_" */
        { gchar *valeur = g_getenv ( *env );                                                             /* Extrait la valeur */
          if (valeur)
           { gchar *env_name = g_ascii_strdown( *env + strlen(prefixe), -1 );                         /* Passage en lowercase */
             Info_new ( __func__, LOG_NOTICE, NULL, "Apply ENV '%s' -> '%s' = '%s'", *env, env_name, valeur );
             if ( !strcasecmp ( valeur, "TRUE" ) )
              { Json_node_add_bool ( target, env_name, TRUE ); }
             else if ( !strcasecmp ( valeur, "FALSE" ) )
              { Json_node_add_bool ( target, env_name, FALSE ); }
             else if ( g_ascii_isdigit ( *valeur ) )         /* Si 1er char. est un digit, on considère qu'il s'agit d'un int */
              { Json_node_add_int  ( target, env_name, atoi(valeur) ); }
             else Json_node_add_string ( target, env_name, valeur );                      /* Sinon d'une chaine de caracteres */
             g_free(env_name);
           }
        }
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
