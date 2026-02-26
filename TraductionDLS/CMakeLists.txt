##################################################################################################################################
# CMakeLists.txt    Profil de compilation DLS pour l'interface API ABLS
# Projet Abls-Habitat version 4.x      Gestion d'habitat
# Auteur: LEFEVRE Sebastien                                                                              26.02.2026
##################################################################################################################################

# Bison: lignes.y -> lignes.c + lignes.h
BISON_TARGET(lignes
  ${CMAKE_CURRENT_SOURCE_DIR}/lignes.y
  ${CMAKE_CURRENT_BINARY_DIR}/lignes.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lignes.h
  COMPILE_FLAGS "-p DlsScanner_ -t -v -Wcounterexamples"
)

# Flex: mots.l -> mots.c (header generated implicitly as mots.h in same dir)
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mots.c
    ${CMAKE_CURRENT_BINARY_DIR}/mots.h
  COMMAND ${FLEX_EXECUTABLE} -Cfa
    --header-file=${CMAKE_CURRENT_BINARY_DIR}/mots.h
    -o ${CMAKE_CURRENT_BINARY_DIR}/mots.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mots.l
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mots.l
  COMMENT "Running flex on mots.l"
)

add_library(tradDLS STATIC
  ${BISON_lignes_OUTPUTS}
  ${CMAKE_CURRENT_BINARY_DIR}/mots.c
  Interp.c
  options.c
  conditions.c
  actions.c
)

target_include_directories(tradDLS PUBLIC
  ${CMAKE_SOURCE_DIR}/Include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${GLIB_INCLUDE_DIRS}
  ${GTHREAD_INCLUDE_DIRS}
  ${JSON_GLIB_INCLUDE_DIRS}
  ${LIBSOUP_INCLUDE_DIRS}
  ${MEMCACHED_INCLUDE_DIRS}
)

target_compile_options(tradDLS PRIVATE
  ${MARIADB_CFLAGS_LIST}
)

target_link_libraries(tradDLS PRIVATE
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${LIBSOUP_LIBRARIES}
  ${MEMCACHED_LIBRARIES}
  ${MARIADB_LIBS_LIST}
)
