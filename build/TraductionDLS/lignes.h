/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_DLSSCANNER_LIGNES_H_INCLUDED
# define YY_DLSSCANNER_LIGNES_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int DlsScanner_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_ERROR = 258,                 /* T_ERROR  */
    PVIRGULE = 259,                /* PVIRGULE  */
    VIRGULE = 260,                 /* VIRGULE  */
    T_DPOINTS = 261,               /* T_DPOINTS  */
    DONNE = 262,                   /* DONNE  */
    EQUIV = 263,                   /* EQUIV  */
    T_MOINS = 264,                 /* T_MOINS  */
    T_POUV = 265,                  /* T_POUV  */
    T_PFERM = 266,                 /* T_PFERM  */
    T_EGAL = 267,                  /* T_EGAL  */
    T_PLUS = 268,                  /* T_PLUS  */
    ET = 269,                      /* ET  */
    BARRE = 270,                   /* BARRE  */
    T_FOIS = 271,                  /* T_FOIS  */
    T_SWITCH = 272,                /* T_SWITCH  */
    T_ACCOUV = 273,                /* T_ACCOUV  */
    T_ACCFERM = 274,               /* T_ACCFERM  */
    T_PIPE = 275,                  /* T_PIPE  */
    T_DIFFERE = 276,               /* T_DIFFERE  */
    T_DEFINE = 277,                /* T_DEFINE  */
    T_LINK = 278,                  /* T_LINK  */
    T_PARAM = 279,                 /* T_PARAM  */
    T_BUS = 280,                   /* T_BUS  */
    T_HOST = 281,                  /* T_HOST  */
    T_TECH_ID = 282,               /* T_TECH_ID  */
    T_COMMANDE = 283,              /* T_COMMANDE  */
    T_MODE = 284,                  /* T_MODE  */
    T_COLOR = 285,                 /* T_COLOR  */
    CLIGNO = 286,                  /* CLIGNO  */
    T_RESET = 287,                 /* T_RESET  */
    T_RW = 288,                    /* T_RW  */
    T_MULTI = 289,                 /* T_MULTI  */
    T_LIBELLE = 290,               /* T_LIBELLE  */
    T_GROUPE = 291,                /* T_GROUPE  */
    T_UNITE = 292,                 /* T_UNITE  */
    T_FORME = 293,                 /* T_FORME  */
    T_DEBUG = 294,                 /* T_DEBUG  */
    T_DISABLE = 295,               /* T_DISABLE  */
    T_PID = 296,                   /* T_PID  */
    T_KP = 297,                    /* T_KP  */
    T_KI = 298,                    /* T_KI  */
    T_KD = 299,                    /* T_KD  */
    T_INPUT = 300,                 /* T_INPUT  */
    T_OUTPUT = 301,                /* T_OUTPUT  */
    T_EXP = 302,                   /* T_EXP  */
    T_ARCSIN = 303,                /* T_ARCSIN  */
    T_ARCTAN = 304,                /* T_ARCTAN  */
    T_ARCCOS = 305,                /* T_ARCCOS  */
    T_SIN = 306,                   /* T_SIN  */
    T_TAN = 307,                   /* T_TAN  */
    T_COS = 308,                   /* T_COS  */
    T_DAA = 309,                   /* T_DAA  */
    T_DMINA = 310,                 /* T_DMINA  */
    T_DMAXA = 311,                 /* T_DMAXA  */
    T_DAD = 312,                   /* T_DAD  */
    T_RANDOM = 313,                /* T_RANDOM  */
    T_CONSIGNE = 314,              /* T_CONSIGNE  */
    T_ALIAS = 315,                 /* T_ALIAS  */
    T_YES = 316,                   /* T_YES  */
    T_NO = 317,                    /* T_NO  */
    T_OVH_ONLY = 318,              /* T_OVH_ONLY  */
    T_TYPE = 319,                  /* T_TYPE  */
    T_ETAT = 320,                  /* T_ETAT  */
    T_NOTIF = 321,                 /* T_NOTIF  */
    T_NOTIF_SMS = 322,             /* T_NOTIF_SMS  */
    T_NOTIF_CHAT = 323,            /* T_NOTIF_CHAT  */
    T_MAP_SMS = 324,               /* T_MAP_SMS  */
    T_BADGE = 325,                 /* T_BADGE  */
    T_DEFAUT = 326,                /* T_DEFAUT  */
    T_ALARME = 327,                /* T_ALARME  */
    T_VEILLE = 328,                /* T_VEILLE  */
    T_ALERTE = 329,                /* T_ALERTE  */
    T_DERANGEMENT = 330,           /* T_DERANGEMENT  */
    T_DANGER = 331,                /* T_DANGER  */
    INF = 332,                     /* INF  */
    SUP = 333,                     /* SUP  */
    INF_OU_EGAL = 334,             /* INF_OU_EGAL  */
    SUP_OU_EGAL = 335,             /* SUP_OU_EGAL  */
    T_TRUE = 336,                  /* T_TRUE  */
    T_FALSE = 337,                 /* T_FALSE  */
    T_NOP = 338,                   /* T_NOP  */
    T_HEURE = 339,                 /* T_HEURE  */
    APRES = 340,                   /* APRES  */
    AVANT = 341,                   /* AVANT  */
    LUNDI = 342,                   /* LUNDI  */
    MARDI = 343,                   /* MARDI  */
    MERCREDI = 344,                /* MERCREDI  */
    JEUDI = 345,                   /* JEUDI  */
    VENDREDI = 346,                /* VENDREDI  */
    SAMEDI = 347,                  /* SAMEDI  */
    DIMANCHE = 348,                /* DIMANCHE  */
    T_BISTABLE = 349,              /* T_BISTABLE  */
    T_MONOSTABLE = 350,            /* T_MONOSTABLE  */
    T_DIGITAL_INPUT = 351,         /* T_DIGITAL_INPUT  */
    T_ANALOG_OUTPUT = 352,         /* T_ANALOG_OUTPUT  */
    T_TEMPO = 353,                 /* T_TEMPO  */
    T_HORLOGE = 354,               /* T_HORLOGE  */
    T_MSG = 355,                   /* T_MSG  */
    T_VISUEL = 356,                /* T_VISUEL  */
    T_CPT_H = 357,                 /* T_CPT_H  */
    T_CPT_IMP = 358,               /* T_CPT_IMP  */
    T_ANALOG_INPUT = 359,          /* T_ANALOG_INPUT  */
    T_START = 360,                 /* T_START  */
    T_REGISTRE = 361,              /* T_REGISTRE  */
    T_DIGITAL_OUTPUT = 362,        /* T_DIGITAL_OUTPUT  */
    T_WATCHDOG = 363,              /* T_WATCHDOG  */
    T_ROUGE = 364,                 /* T_ROUGE  */
    T_VERT = 365,                  /* T_VERT  */
    T_BLEU = 366,                  /* T_BLEU  */
    T_JAUNE = 367,                 /* T_JAUNE  */
    T_NOIR = 368,                  /* T_NOIR  */
    T_BLANC = 369,                 /* T_BLANC  */
    T_ORANGE = 370,                /* T_ORANGE  */
    T_GRIS = 371,                  /* T_GRIS  */
    T_KAKI = 372,                  /* T_KAKI  */
    T_CYAN = 373,                  /* T_CYAN  */
    T_EDGE_UP = 374,               /* T_EDGE_UP  */
    T_EDGE_DOWN = 375,             /* T_EDGE_DOWN  */
    T_IN_RANGE = 376,              /* T_IN_RANGE  */
    T_MIN = 377,                   /* T_MIN  */
    T_MAX = 378,                   /* T_MAX  */
    T_SEUIL_NTB = 379,             /* T_SEUIL_NTB  */
    T_SEUIL_NB = 380,              /* T_SEUIL_NB  */
    T_SEUIL_NH = 381,              /* T_SEUIL_NH  */
    T_SEUIL_NTH = 382,             /* T_SEUIL_NTH  */
    T_DECIMAL = 383,               /* T_DECIMAL  */
    T_NOSHOW = 384,                /* T_NOSHOW  */
    T_FREEZE = 385,                /* T_FREEZE  */
    T_CHAINE = 386,                /* T_CHAINE  */
    ID = 387,                      /* ID  */
    ENTIER = 388,                  /* ENTIER  */
    T_VALF = 389                   /* T_VALF  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 46 "/home/sebastien/API/TraductionDLS/lignes.y"
 gint val;
         gdouble valf;
         gchar *chaine;
         GList *gliste;
         struct OPTION *option;
         struct ACTION *action;
         struct ALIAS *t_alias;
         struct CONDITION *t_condition;
         struct INSTRUCTION *t_instruction;
       

#line 210 "lignes.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int DlsScanner_parse (void *scan_instance);


#endif /* !YY_DLSSCANNER_LIGNES_H_INCLUDED  */
