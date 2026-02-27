/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         DlsScanner_parse
#define yylex           DlsScanner_lex
#define yyerror         DlsScanner_error
#define yydebug         DlsScanner_debug
#define yynerrs         DlsScanner_nerrs

/* First part of user prologue.  */
#line 28 "/home/sebastien/API/TraductionDLS/lignes.y"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "Http.h"
#include "Dls_trad.h"


#line 86 "/home/sebastien/API/build/TraductionDLS/lignes.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lignes.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_T_ERROR = 3,                    /* T_ERROR  */
  YYSYMBOL_PVIRGULE = 4,                   /* PVIRGULE  */
  YYSYMBOL_VIRGULE = 5,                    /* VIRGULE  */
  YYSYMBOL_T_DPOINTS = 6,                  /* T_DPOINTS  */
  YYSYMBOL_DONNE = 7,                      /* DONNE  */
  YYSYMBOL_EQUIV = 8,                      /* EQUIV  */
  YYSYMBOL_T_MOINS = 9,                    /* T_MOINS  */
  YYSYMBOL_T_POUV = 10,                    /* T_POUV  */
  YYSYMBOL_T_PFERM = 11,                   /* T_PFERM  */
  YYSYMBOL_T_EGAL = 12,                    /* T_EGAL  */
  YYSYMBOL_T_PLUS = 13,                    /* T_PLUS  */
  YYSYMBOL_ET = 14,                        /* ET  */
  YYSYMBOL_BARRE = 15,                     /* BARRE  */
  YYSYMBOL_T_FOIS = 16,                    /* T_FOIS  */
  YYSYMBOL_T_SWITCH = 17,                  /* T_SWITCH  */
  YYSYMBOL_T_ACCOUV = 18,                  /* T_ACCOUV  */
  YYSYMBOL_T_ACCFERM = 19,                 /* T_ACCFERM  */
  YYSYMBOL_T_PIPE = 20,                    /* T_PIPE  */
  YYSYMBOL_T_DIFFERE = 21,                 /* T_DIFFERE  */
  YYSYMBOL_T_DEFINE = 22,                  /* T_DEFINE  */
  YYSYMBOL_T_LINK = 23,                    /* T_LINK  */
  YYSYMBOL_T_PARAM = 24,                   /* T_PARAM  */
  YYSYMBOL_T_BUS = 25,                     /* T_BUS  */
  YYSYMBOL_T_HOST = 26,                    /* T_HOST  */
  YYSYMBOL_T_TECH_ID = 27,                 /* T_TECH_ID  */
  YYSYMBOL_T_COMMANDE = 28,                /* T_COMMANDE  */
  YYSYMBOL_T_MODE = 29,                    /* T_MODE  */
  YYSYMBOL_T_COLOR = 30,                   /* T_COLOR  */
  YYSYMBOL_CLIGNO = 31,                    /* CLIGNO  */
  YYSYMBOL_T_RESET = 32,                   /* T_RESET  */
  YYSYMBOL_T_RW = 33,                      /* T_RW  */
  YYSYMBOL_T_MULTI = 34,                   /* T_MULTI  */
  YYSYMBOL_T_LIBELLE = 35,                 /* T_LIBELLE  */
  YYSYMBOL_T_GROUPE = 36,                  /* T_GROUPE  */
  YYSYMBOL_T_UNITE = 37,                   /* T_UNITE  */
  YYSYMBOL_T_FORME = 38,                   /* T_FORME  */
  YYSYMBOL_T_DEBUG = 39,                   /* T_DEBUG  */
  YYSYMBOL_T_DISABLE = 40,                 /* T_DISABLE  */
  YYSYMBOL_T_PID = 41,                     /* T_PID  */
  YYSYMBOL_T_KP = 42,                      /* T_KP  */
  YYSYMBOL_T_KI = 43,                      /* T_KI  */
  YYSYMBOL_T_KD = 44,                      /* T_KD  */
  YYSYMBOL_T_INPUT = 45,                   /* T_INPUT  */
  YYSYMBOL_T_OUTPUT = 46,                  /* T_OUTPUT  */
  YYSYMBOL_T_EXP = 47,                     /* T_EXP  */
  YYSYMBOL_T_ARCSIN = 48,                  /* T_ARCSIN  */
  YYSYMBOL_T_ARCTAN = 49,                  /* T_ARCTAN  */
  YYSYMBOL_T_ARCCOS = 50,                  /* T_ARCCOS  */
  YYSYMBOL_T_SIN = 51,                     /* T_SIN  */
  YYSYMBOL_T_TAN = 52,                     /* T_TAN  */
  YYSYMBOL_T_COS = 53,                     /* T_COS  */
  YYSYMBOL_T_DAA = 54,                     /* T_DAA  */
  YYSYMBOL_T_DMINA = 55,                   /* T_DMINA  */
  YYSYMBOL_T_DMAXA = 56,                   /* T_DMAXA  */
  YYSYMBOL_T_DAD = 57,                     /* T_DAD  */
  YYSYMBOL_T_RANDOM = 58,                  /* T_RANDOM  */
  YYSYMBOL_T_CONSIGNE = 59,                /* T_CONSIGNE  */
  YYSYMBOL_T_ALIAS = 60,                   /* T_ALIAS  */
  YYSYMBOL_T_YES = 61,                     /* T_YES  */
  YYSYMBOL_T_NO = 62,                      /* T_NO  */
  YYSYMBOL_T_OVH_ONLY = 63,                /* T_OVH_ONLY  */
  YYSYMBOL_T_TYPE = 64,                    /* T_TYPE  */
  YYSYMBOL_T_ETAT = 65,                    /* T_ETAT  */
  YYSYMBOL_T_NOTIF = 66,                   /* T_NOTIF  */
  YYSYMBOL_T_NOTIF_SMS = 67,               /* T_NOTIF_SMS  */
  YYSYMBOL_T_NOTIF_CHAT = 68,              /* T_NOTIF_CHAT  */
  YYSYMBOL_T_MAP_SMS = 69,                 /* T_MAP_SMS  */
  YYSYMBOL_T_BADGE = 70,                   /* T_BADGE  */
  YYSYMBOL_T_DEFAUT = 71,                  /* T_DEFAUT  */
  YYSYMBOL_T_ALARME = 72,                  /* T_ALARME  */
  YYSYMBOL_T_VEILLE = 73,                  /* T_VEILLE  */
  YYSYMBOL_T_ALERTE = 74,                  /* T_ALERTE  */
  YYSYMBOL_T_DERANGEMENT = 75,             /* T_DERANGEMENT  */
  YYSYMBOL_T_DANGER = 76,                  /* T_DANGER  */
  YYSYMBOL_INF = 77,                       /* INF  */
  YYSYMBOL_SUP = 78,                       /* SUP  */
  YYSYMBOL_INF_OU_EGAL = 79,               /* INF_OU_EGAL  */
  YYSYMBOL_SUP_OU_EGAL = 80,               /* SUP_OU_EGAL  */
  YYSYMBOL_T_TRUE = 81,                    /* T_TRUE  */
  YYSYMBOL_T_FALSE = 82,                   /* T_FALSE  */
  YYSYMBOL_T_NOP = 83,                     /* T_NOP  */
  YYSYMBOL_T_HEURE = 84,                   /* T_HEURE  */
  YYSYMBOL_APRES = 85,                     /* APRES  */
  YYSYMBOL_AVANT = 86,                     /* AVANT  */
  YYSYMBOL_LUNDI = 87,                     /* LUNDI  */
  YYSYMBOL_MARDI = 88,                     /* MARDI  */
  YYSYMBOL_MERCREDI = 89,                  /* MERCREDI  */
  YYSYMBOL_JEUDI = 90,                     /* JEUDI  */
  YYSYMBOL_VENDREDI = 91,                  /* VENDREDI  */
  YYSYMBOL_SAMEDI = 92,                    /* SAMEDI  */
  YYSYMBOL_DIMANCHE = 93,                  /* DIMANCHE  */
  YYSYMBOL_T_BISTABLE = 94,                /* T_BISTABLE  */
  YYSYMBOL_T_MONOSTABLE = 95,              /* T_MONOSTABLE  */
  YYSYMBOL_T_DIGITAL_INPUT = 96,           /* T_DIGITAL_INPUT  */
  YYSYMBOL_T_ANALOG_OUTPUT = 97,           /* T_ANALOG_OUTPUT  */
  YYSYMBOL_T_TEMPO = 98,                   /* T_TEMPO  */
  YYSYMBOL_T_HORLOGE = 99,                 /* T_HORLOGE  */
  YYSYMBOL_T_MSG = 100,                    /* T_MSG  */
  YYSYMBOL_T_VISUEL = 101,                 /* T_VISUEL  */
  YYSYMBOL_T_CPT_H = 102,                  /* T_CPT_H  */
  YYSYMBOL_T_CPT_IMP = 103,                /* T_CPT_IMP  */
  YYSYMBOL_T_ANALOG_INPUT = 104,           /* T_ANALOG_INPUT  */
  YYSYMBOL_T_START = 105,                  /* T_START  */
  YYSYMBOL_T_REGISTRE = 106,               /* T_REGISTRE  */
  YYSYMBOL_T_DIGITAL_OUTPUT = 107,         /* T_DIGITAL_OUTPUT  */
  YYSYMBOL_T_WATCHDOG = 108,               /* T_WATCHDOG  */
  YYSYMBOL_T_ROUGE = 109,                  /* T_ROUGE  */
  YYSYMBOL_T_VERT = 110,                   /* T_VERT  */
  YYSYMBOL_T_BLEU = 111,                   /* T_BLEU  */
  YYSYMBOL_T_JAUNE = 112,                  /* T_JAUNE  */
  YYSYMBOL_T_NOIR = 113,                   /* T_NOIR  */
  YYSYMBOL_T_BLANC = 114,                  /* T_BLANC  */
  YYSYMBOL_T_ORANGE = 115,                 /* T_ORANGE  */
  YYSYMBOL_T_GRIS = 116,                   /* T_GRIS  */
  YYSYMBOL_T_KAKI = 117,                   /* T_KAKI  */
  YYSYMBOL_T_CYAN = 118,                   /* T_CYAN  */
  YYSYMBOL_T_EDGE_UP = 119,                /* T_EDGE_UP  */
  YYSYMBOL_T_EDGE_DOWN = 120,              /* T_EDGE_DOWN  */
  YYSYMBOL_T_IN_RANGE = 121,               /* T_IN_RANGE  */
  YYSYMBOL_T_MIN = 122,                    /* T_MIN  */
  YYSYMBOL_T_MAX = 123,                    /* T_MAX  */
  YYSYMBOL_T_SEUIL_NTB = 124,              /* T_SEUIL_NTB  */
  YYSYMBOL_T_SEUIL_NB = 125,               /* T_SEUIL_NB  */
  YYSYMBOL_T_SEUIL_NH = 126,               /* T_SEUIL_NH  */
  YYSYMBOL_T_SEUIL_NTH = 127,              /* T_SEUIL_NTH  */
  YYSYMBOL_T_DECIMAL = 128,                /* T_DECIMAL  */
  YYSYMBOL_T_NOSHOW = 129,                 /* T_NOSHOW  */
  YYSYMBOL_T_FREEZE = 130,                 /* T_FREEZE  */
  YYSYMBOL_T_CHAINE = 131,                 /* T_CHAINE  */
  YYSYMBOL_ID = 132,                       /* ID  */
  YYSYMBOL_ENTIER = 133,                   /* ENTIER  */
  YYSYMBOL_T_VALF = 134,                   /* T_VALF  */
  YYSYMBOL_YYACCEPT = 135,                 /* $accept  */
  YYSYMBOL_fichier = 136,                  /* fichier  */
  YYSYMBOL_listeDefinitions = 137,         /* listeDefinitions  */
  YYSYMBOL_une_definition = 138,           /* une_definition  */
  YYSYMBOL_alias_classe = 139,             /* alias_classe  */
  YYSYMBOL_listeInstr = 140,               /* listeInstr  */
  YYSYMBOL_une_instr = 141,                /* une_instr  */
  YYSYMBOL_un_switch = 142,                /* un_switch  */
  YYSYMBOL_listeCase = 143,                /* listeCase  */
  YYSYMBOL_expr = 144,                     /* expr  */
  YYSYMBOL_unite = 145,                    /* unite  */
  YYSYMBOL_liste_action = 146,             /* liste_action  */
  YYSYMBOL_une_action = 147,               /* une_action  */
  YYSYMBOL_barre = 148,                    /* barre  */
  YYSYMBOL_jour_semaine = 149,             /* jour_semaine  */
  YYSYMBOL_ordre = 150,                    /* ordre  */
  YYSYMBOL_liste_options = 151,            /* liste_options  */
  YYSYMBOL_options = 152,                  /* options  */
  YYSYMBOL_une_option = 153,               /* une_option  */
  YYSYMBOL_couleur = 154,                  /* couleur  */
  YYSYMBOL_type_msg = 155,                 /* type_msg  */
  YYSYMBOL_type_notif_sms = 156,           /* type_notif_sms  */
  YYSYMBOL_type_notif_chat = 157,          /* type_notif_chat  */
  YYSYMBOL_un_alias = 158                  /* un_alias  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   461

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  135
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  24
/* YYNRULES -- Number of rules.  */
#define YYNRULES  162
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  306

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   389


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   111,   111,   124,   125,   128,   136,   143,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   169,   244,   253,   256,   258,   260,   270,
     283,   299,   311,   326,   340,   353,   366,   380,   391,   396,
     399,   403,   404,   405,   413,   421,   429,   437,   445,   453,
     461,   482,   486,   490,   494,   500,   517,   524,   526,   530,
     573,   574,   576,   577,   578,   579,   580,   581,   582,   584,
     584,   584,   584,   584,   587,   588,   589,   592,   594,   597,
     603,   609,   615,   621,   627,   633,   639,   645,   652,   658,
     664,   670,   676,   682,   688,   694,   701,   707,   713,   719,
     725,   731,   737,   743,   749,   755,   761,   767,   773,   779,
     785,   791,   797,   803,   809,   815,   821,   827,   833,   839,
     845,   851,   857,   863,   869,   875,   881,   887,   893,   899,
     905,   911,   917,   923,   929,   935,   941,   947,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   966,   967,
     968,   969,   970,   971,   972,   973,   976,   976,   976,   977,
     977,   979,   989
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "T_ERROR", "PVIRGULE",
  "VIRGULE", "T_DPOINTS", "DONNE", "EQUIV", "T_MOINS", "T_POUV", "T_PFERM",
  "T_EGAL", "T_PLUS", "ET", "BARRE", "T_FOIS", "T_SWITCH", "T_ACCOUV",
  "T_ACCFERM", "T_PIPE", "T_DIFFERE", "T_DEFINE", "T_LINK", "T_PARAM",
  "T_BUS", "T_HOST", "T_TECH_ID", "T_COMMANDE", "T_MODE", "T_COLOR",
  "CLIGNO", "T_RESET", "T_RW", "T_MULTI", "T_LIBELLE", "T_GROUPE",
  "T_UNITE", "T_FORME", "T_DEBUG", "T_DISABLE", "T_PID", "T_KP", "T_KI",
  "T_KD", "T_INPUT", "T_OUTPUT", "T_EXP", "T_ARCSIN", "T_ARCTAN",
  "T_ARCCOS", "T_SIN", "T_TAN", "T_COS", "T_DAA", "T_DMINA", "T_DMAXA",
  "T_DAD", "T_RANDOM", "T_CONSIGNE", "T_ALIAS", "T_YES", "T_NO",
  "T_OVH_ONLY", "T_TYPE", "T_ETAT", "T_NOTIF", "T_NOTIF_SMS",
  "T_NOTIF_CHAT", "T_MAP_SMS", "T_BADGE", "T_DEFAUT", "T_ALARME",
  "T_VEILLE", "T_ALERTE", "T_DERANGEMENT", "T_DANGER", "INF", "SUP",
  "INF_OU_EGAL", "SUP_OU_EGAL", "T_TRUE", "T_FALSE", "T_NOP", "T_HEURE",
  "APRES", "AVANT", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI",
  "SAMEDI", "DIMANCHE", "T_BISTABLE", "T_MONOSTABLE", "T_DIGITAL_INPUT",
  "T_ANALOG_OUTPUT", "T_TEMPO", "T_HORLOGE", "T_MSG", "T_VISUEL",
  "T_CPT_H", "T_CPT_IMP", "T_ANALOG_INPUT", "T_START", "T_REGISTRE",
  "T_DIGITAL_OUTPUT", "T_WATCHDOG", "T_ROUGE", "T_VERT", "T_BLEU",
  "T_JAUNE", "T_NOIR", "T_BLANC", "T_ORANGE", "T_GRIS", "T_KAKI", "T_CYAN",
  "T_EDGE_UP", "T_EDGE_DOWN", "T_IN_RANGE", "T_MIN", "T_MAX",
  "T_SEUIL_NTB", "T_SEUIL_NB", "T_SEUIL_NH", "T_SEUIL_NTH", "T_DECIMAL",
  "T_NOSHOW", "T_FREEZE", "T_CHAINE", "ID", "ENTIER", "T_VALF", "$accept",
  "fichier", "listeDefinitions", "une_definition", "alias_classe",
  "listeInstr", "une_instr", "un_switch", "listeCase", "expr", "unite",
  "liste_action", "une_action", "barre", "jour_semaine", "ordre",
  "liste_options", "options", "une_option", "couleur", "type_msg",
  "type_notif_sms", "type_notif_chat", "un_alias", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-200)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -11,  -116,  -110,  -101,     8,     1,   -11,    34,    48,    64,
    -200,   168,    96,  -200,     1,     1,  -200,    11,    -7,   118,
     116,  -200,   128,   149,   160,   171,   174,   182,   183,  -200,
    -200,    57,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,    63,  -200,    -5,  -200,   119,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,    64,    64,  -200,   195,   196,
     210,   211,   212,   213,   214,  -200,   215,   224,   239,   241,
     242,  -200,  -200,   254,   259,   260,   262,   278,   288,   297,
     298,   299,   300,   302,   303,   310,   318,   326,   334,   342,
    -200,  -200,  -200,   349,   350,   351,   352,   353,   354,   355,
    -200,   368,     4,  -200,  -200,   168,   168,   168,   168,   168,
     168,   168,  -200,  -200,  -200,  -200,  -200,    62,     2,   168,
     168,   168,   168,   168,   249,   168,   168,   375,    64,    -1,
      96,   378,   379,   253,   255,   256,   257,   316,   252,   261,
     267,   264,   263,   271,   272,   277,   277,   277,   277,   277,
     279,   284,   285,   286,   287,   -88,   138,   -33,     0,   273,
     280,  -112,   -68,   -39,   -66,   -35,    -3,    32,   311,   313,
     249,  -200,    44,   185,   219,   312,   320,   328,   336,   387,
       1,    64,  -200,   163,  -200,   277,    23,    23,   190,  -200,
    -200,    27,  -200,   344,   317,  -200,    56,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,
    -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,  -200,   315,
     391,  -200,  -200,    56,    64,    56,  -200,  -200,   175,  -200,
    -200,  -200,  -200,   186,  -200,  -200
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     0,     0,    25,     4,     0,     0,    76,
       1,    61,     0,     2,    25,    25,     3,     0,     0,     0,
       0,    60,     0,     0,     0,     0,     0,     0,     0,    53,
      54,     0,    62,    63,    64,    65,    66,    67,    68,    52,
      42,    41,     0,    39,     0,    51,     0,    29,    23,    24,
      21,     8,     9,    10,    17,    12,    20,    11,    13,    14,
      15,    16,    19,    18,    22,    76,    76,    75,     0,     0,
       0,     0,     0,    97,   101,   102,     0,     0,     0,     0,
       0,    99,   100,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   134,   136,     0,     0,     0,
      85,    86,    87,     0,     0,     0,     0,     0,     0,     0,
     124,     0,     0,    78,     7,    61,    61,    61,    61,    61,
      61,    61,    73,    69,    70,    71,    72,     0,    61,    61,
      61,    61,    61,    61,     0,    61,    61,   161,    76,    61,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    76,    57,     0,    56,     0,    33,    32,    34,    36,
      35,     0,    38,     0,     0,    40,    61,    30,     5,     6,
      88,    89,    90,    91,   138,   139,   140,   141,   142,   143,
     145,   144,   146,   147,    95,    94,    98,   103,   104,    81,
      80,    84,    96,   129,   130,   131,   127,   128,   106,   107,
     108,   109,   110,    79,   126,   148,   149,   150,   151,   152,
     153,   155,   154,   105,   156,   157,   158,   135,   159,   160,
     137,    93,    92,    82,    83,   111,   112,   132,   113,   114,
     133,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     125,    77,    43,    44,    45,    46,    47,    48,    49,     0,
       0,    58,    26,    61,    76,    61,    37,   162,     0,    50,
      28,    55,    59,     0,    31,    27
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -200,  -200,   429,  -200,  -200,   -12,   397,  -200,   305,   321,
    -200,  -199,   162,  -124,  -200,   427,   -65,   325,   281,  -200,
    -200,  -200,  -200,  -132
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     4,     5,     6,    65,    13,    14,    15,    47,    42,
      43,   193,   194,    44,    45,   135,    20,   112,   113,   225,
     253,   257,   260,   138
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     141,   142,    48,    49,   195,   136,   206,   298,    10,   180,
      11,     1,     2,     3,    21,   181,     7,    21,    12,   263,
     190,   264,     8,   233,   234,   235,   236,   237,   254,   255,
     256,     9,   180,   244,   295,   122,    50,   131,   132,   133,
     267,   270,    17,   191,   137,   243,    22,    23,    24,    25,
      26,    27,    28,   129,    18,   282,   122,   130,   131,   132,
     133,   258,   259,   294,   137,   265,   266,   271,   272,   122,
     128,    21,   129,   205,    19,   122,   130,   131,   132,   133,
      29,    30,   195,    31,   134,   192,    32,    33,    34,    35,
      36,    37,    38,   137,   268,   269,   303,   191,   273,   274,
     123,   124,   125,   126,    39,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    46,    62,    63,    64,
     114,   123,   124,   125,   126,    66,   291,   137,   139,    67,
     275,   276,    40,    41,   123,   124,   125,   126,   115,   192,
     123,   124,   125,   126,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,   116,
      83,    84,    85,    86,    87,   277,   278,   292,   293,   195,
     117,   195,    88,    89,    90,    91,    92,    93,   290,   304,
     293,   118,    94,    21,   119,    95,    96,    97,    98,    99,
     305,   293,   120,   121,   129,   189,   283,   122,   130,   131,
     132,   133,   122,   245,   246,   132,   133,   143,   144,   247,
     248,   249,   250,   251,   252,    22,    23,    24,    25,    26,
      27,    28,   145,   146,   147,   148,   149,   150,   129,   302,
     284,   122,   130,   131,   132,   133,   151,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    29,
      30,   152,    31,   153,   154,    32,    33,    34,    35,    36,
      37,    38,   123,   124,   125,   126,   155,   123,   124,   125,
     126,   156,   157,    39,   158,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
     159,    83,    84,    85,    86,    87,   123,   124,   125,   126,
     160,    40,    41,    88,    89,    90,    91,    92,    93,   161,
     162,   163,   164,    94,   165,   166,    95,    96,    97,    98,
      99,   129,   167,   285,   122,   130,   131,   132,   133,   129,
     168,   286,   122,   130,   131,   132,   133,   129,   169,   287,
     122,   130,   131,   132,   133,   129,   170,   288,   122,   130,
     131,   132,   133,   129,   171,   296,   122,   130,   131,   132,
     133,   172,   173,   174,   175,   176,   177,   178,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     179,   204,   208,   209,   210,   226,   211,   212,   213,   123,
     124,   125,   126,   289,   227,   229,   230,   123,   124,   125,
     126,   228,   231,   232,   261,   123,   124,   125,   126,   137,
     300,   262,   238,   123,   124,   125,   126,   239,   240,   241,
     242,   123,   124,   125,   126,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,    16,   182,   183,   184,   185,
     186,   187,   188,   140,   279,   207,   280,   224,   299,   297,
     196,   197,   198,   199,   200,   301,   202,   203,   127,   201,
       0,   281
};

static const yytype_int16 yycheck[] =
{
      65,    66,    14,    15,   128,    10,     7,   206,     0,     5,
       9,    22,    23,    24,    15,    11,   132,    15,    17,   131,
      18,   133,   132,   155,   156,   157,   158,   159,    61,    62,
      63,   132,     5,   165,     7,    12,    25,    14,    15,    16,
     172,   173,     8,    41,   132,   133,    47,    48,    49,    50,
      51,    52,    53,     9,     6,    11,    12,    13,    14,    15,
      16,    61,    62,   195,   132,   133,   134,   133,   134,    12,
       7,    15,     9,   138,    10,    12,    13,    14,    15,    16,
      81,    82,   206,    84,    21,    83,    87,    88,    89,    90,
      91,    92,    93,   132,   133,   134,   295,    41,   133,   134,
      77,    78,    79,    80,   105,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    20,   106,   107,   108,
       4,    77,    78,    79,    80,   132,   191,   132,     9,    11,
     133,   134,   133,   134,    77,    78,    79,    80,    10,    83,
      77,    78,    79,    80,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    10,
      42,    43,    44,    45,    46,   133,   134,     4,     5,   293,
      10,   295,    54,    55,    56,    57,    58,    59,   190,     4,
       5,    10,    64,    15,    10,    67,    68,    69,    70,    71,
       4,     5,    10,    10,     9,   133,    11,    12,    13,    14,
      15,    16,    12,    65,    66,    15,    16,    12,    12,    71,
      72,    73,    74,    75,    76,    47,    48,    49,    50,    51,
      52,    53,    12,    12,    12,    12,    12,    12,     9,   294,
      11,    12,    13,    14,    15,    16,    12,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,    81,
      82,    12,    84,    12,    12,    87,    88,    89,    90,    91,
      92,    93,    77,    78,    79,    80,    12,    77,    78,    79,
      80,    12,    12,   105,    12,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      12,    42,    43,    44,    45,    46,    77,    78,    79,    80,
      12,   133,   134,    54,    55,    56,    57,    58,    59,    12,
      12,    12,    12,    64,    12,    12,    67,    68,    69,    70,
      71,     9,    12,    11,    12,    13,    14,    15,    16,     9,
      12,    11,    12,    13,    14,    15,    16,     9,    12,    11,
      12,    13,    14,    15,    16,     9,    12,    11,    12,    13,
      14,    15,    16,     9,    12,    11,    12,    13,    14,    15,
      16,    12,    12,    12,    12,    12,    12,    12,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
      12,     6,     4,     4,   131,   133,   131,   131,   131,    77,
      78,    79,    80,     6,   133,   131,   133,    77,    78,    79,
      80,   134,   131,   131,   131,    77,    78,    79,    80,   132,
      19,   131,   133,    77,    78,    79,    80,   133,   133,   133,
     133,    77,    78,    79,    80,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,     6,   115,   116,   117,   118,
     119,   120,   121,    46,   133,   140,   133,   131,   133,   132,
     129,   130,   131,   132,   133,   293,   135,   136,    31,   134,
      -1,   180
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    22,    23,    24,   136,   137,   138,   132,   132,   132,
       0,     9,    17,   140,   141,   142,   137,     8,     6,    10,
     151,    15,    47,    48,    49,    50,    51,    52,    53,    81,
      82,    84,    87,    88,    89,    90,    91,    92,    93,   105,
     133,   134,   144,   145,   148,   149,    20,   143,   140,   140,
      25,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   106,   107,   108,   139,   132,    11,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    42,    43,    44,    45,    46,    54,    55,
      56,    57,    58,    59,    64,    67,    68,    69,    70,    71,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   152,   153,     4,    10,    10,    10,    10,    10,
      10,    10,    12,    77,    78,    79,    80,   150,     7,     9,
      13,    14,    15,    16,    21,   150,    10,   132,   158,     9,
     141,   151,   151,    12,    12,    12,    12,    12,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
       5,    11,   144,   144,   144,   144,   144,   144,   144,   133,
      18,    41,    83,   146,   147,   148,   144,   144,   144,   144,
     144,   152,   144,   144,     6,   151,     7,   143,     4,     4,
     131,   131,   131,   131,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   131,   154,   133,   133,   134,   131,
     133,   131,   131,   158,   158,   158,   158,   158,   133,   133,
     133,   133,   133,   133,   158,    65,    66,    71,    72,    73,
      74,    75,    76,   155,    61,    62,    63,   156,    61,    62,
     157,   131,   131,   131,   133,   133,   134,   158,   133,   134,
     158,   133,   134,   133,   134,   133,   134,   133,   134,   133,
     133,   153,    11,    11,    11,    11,    11,    11,    11,     6,
     140,   151,     4,     5,   158,     7,    11,   132,   146,   133,
      19,   147,   151,   146,     4,     4
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   135,   136,   137,   137,   138,   138,   138,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   140,   140,   140,   141,   141,   141,   142,
     143,   143,   144,   144,   144,   144,   144,   144,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   146,   146,   147,   147,   147,
     148,   148,   149,   149,   149,   149,   149,   149,   149,   150,
     150,   150,   150,   150,   151,   151,   151,   152,   152,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   155,   155,
     155,   155,   155,   155,   155,   155,   156,   156,   156,   157,
     157,   158,   158
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     2,     0,     6,     6,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     5,     7,     6,     2,
       3,     5,     3,     3,     3,     3,     3,     4,     3,     1,
       3,     1,     1,     4,     4,     4,     4,     4,     4,     4,
       5,     1,     1,     1,     1,     3,     1,     1,     2,     3,
       1,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     0,     3,     1,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     3,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scan_instance, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scan_instance); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scan_instance)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scan_instance);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scan_instance)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scan_instance);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scan_instance)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scan_instance);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scan_instance); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scan_instance)
{
  YY_USE (yyvaluep);
  YY_USE (scan_instance);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scan_instance)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scan_instance);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* fichier: listeDefinitions listeInstr  */
#line 112 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ gchar *Start_Go = " void Go ( struct DLS_TO_PLUGIN *vars )\n"
                                     "  {\n";
                   Emettre( scan_instance, Start_Go );
                   if((yyvsp[0].chaine)) { Emettre( scan_instance, (yyvsp[0].chaine) ); g_free((yyvsp[0].chaine)); }
/*----------------------------------------------- Ecriture de la fin de fonction Go ------------------------------------------*/
                   Add_unused_as_action_visuels ( scan_instance );
                   gchar *End_Go =   "  }\n";
                   Emettre( scan_instance, End_Go );                                                  /* Ecriture du prologue */
                }}
#line 1776 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 4: /* listeDefinitions: %empty  */
#line 125 "/home/sebastien/API/TraductionDLS/lignes.y"
                  {{ }}
#line 1782 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 5: /* une_definition: T_DEFINE ID EQUIV alias_classe liste_options PVIRGULE  */
#line 129 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ( Get_local_alias(scan_instance, NULL, (yyvsp[-4].chaine)) )                                           /* Deja defini ? */
                        { Emettre_erreur_new( scan_instance, "'%s' is already defined", (yyvsp[-4].chaine) );
                          Liberer_options((yyvsp[-1].gliste));
                        }
                   else { New_alias( scan_instance, NULL, (yyvsp[-4].chaine), (yyvsp[-2].val), (yyvsp[-1].gliste) ); }
                   g_free((yyvsp[-4].chaine));
                }}
#line 1794 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 6: /* une_definition: T_LINK ID T_DPOINTS ID liste_options PVIRGULE  */
#line 137 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-4].chaine) && (yyvsp[-2].chaine))
                    { New_link ( scan_instance, (yyvsp[-4].chaine), (yyvsp[-2].chaine), (yyvsp[-1].gliste) ); }                                         /* Création d'un link */
                   Liberer_options((yyvsp[-1].gliste));
                   if ((yyvsp[-4].chaine)) g_free((yyvsp[-4].chaine));
                   if ((yyvsp[-2].chaine)) g_free((yyvsp[-2].chaine));
                }}
#line 1805 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 7: /* une_definition: T_PARAM ID liste_options PVIRGULE  */
#line 144 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].chaine))
                    { New_parametre ( scan_instance, (yyvsp[-2].chaine), (yyvsp[-1].gliste) ); }                                   /* Création d'un parametre */
                   Liberer_options((yyvsp[-1].gliste));
                   if ((yyvsp[-2].chaine)) g_free((yyvsp[-2].chaine));
                }}
#line 1815 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 23: /* listeInstr: une_instr listeInstr  */
#line 170 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].t_instruction) && (yyvsp[-1].t_instruction)->condition->is_bool == FALSE) /* Si la condition est arithmétique */
                    { gint taille = (yyvsp[-1].t_instruction)->condition->taille + (yyvsp[-1].t_instruction)->actions->taille_alors + ((yyvsp[0].chaine) ? strlen((yyvsp[0].chaine)) : 0) + 256;
                      (yyval.chaine) = New_chaine( taille );
                      g_snprintf( (yyval.chaine), taille,
                                  "/* -%06d----------une_instr FLOAT-------*/\n"
                                  "vars->num_ligne = %d;\n"
                                  " { gdouble local_result=%s;\n"
                                  "   %s\n"
                                  " }\n%s", taille, (yyvsp[-1].t_instruction)->line_number, (yyvsp[-1].t_instruction)->condition->chaine, (yyvsp[-1].t_instruction)->actions->alors, ((yyvsp[0].chaine) ? (yyvsp[0].chaine) : "/**/") );
                    }
                   else if ((yyvsp[-1].t_instruction) && (yyvsp[-1].t_instruction)->condition->is_bool == TRUE) /* Si la condition est booléenne */
                    { gint taille  = (yyvsp[-1].t_instruction)->condition->taille + (yyvsp[-1].t_instruction)->actions->taille_alors + (yyvsp[-1].t_instruction)->actions->taille_sinon + ((yyvsp[0].chaine) ? strlen((yyvsp[0].chaine)) : 0) + 256;
                      gchar *sinon = ((yyvsp[-1].t_instruction)->actions->sinon ? (yyvsp[-1].t_instruction)->actions->sinon : "/* no sinon action */");
                      if ( Get_option_entier((yyvsp[-1].t_instruction)->options, T_DAA, 0) || Get_option_entier((yyvsp[-1].t_instruction)->options, T_DAD, 0) )
                       { taille +=1024;
                         (yyval.chaine) = New_chaine( taille );
                         g_snprintf( (yyval.chaine), taille,
                                     "/* -%06d-------------une_instr différée----------*/\n"
                                     "vars->num_ligne = %d;\n"
                                     " { static gint prev_state = -1;\n"
                                     "   static gboolean counting_on=FALSE;\n"
                                     "   static gboolean counting_off=FALSE;\n"
                                     "   static gint top;\n"
                                     "   if(%s)\n"
                                     "    { counting_off=FALSE;\n"
                                     "      if (counting_on==FALSE)\n"
                                     "       { counting_on=TRUE; top = Dls_get_top(); }\n"
                                     "      else\n"
                                     "       { if ( Dls_get_top() - top >= %d )\n"
                                     "          { %s; prev_state = 1;\n"
                                     "          }\n"
                                     "       }\n"
                                     "    }\n"
                                     "   else\n"
                                     "    { counting_on = FALSE;\n"
                                     "      if (counting_off==FALSE)\n"
                                     "       { counting_off=TRUE; top = Dls_get_top(); }\n"
                                     "      else\n"
                                     "       { if ( Dls_get_top() - top >= %d )\n"
                                     "          { %s; prev_state = 0;\n"
                                     "          }\n"
                                     "       }\n"
                                     "    }\n"
                                     " }\n\n%s",
                                     taille, (yyvsp[-1].t_instruction)->line_number, (yyvsp[-1].t_instruction)->condition->chaine,
                                     Get_option_entier((yyvsp[-1].t_instruction)->options, T_DAA, 0), (yyvsp[-1].t_instruction)->actions->alors,
                                     Get_option_entier((yyvsp[-1].t_instruction)->options, T_DAD, 0), sinon, ((yyvsp[0].chaine) ? (yyvsp[0].chaine) : "/**/") );
                       }
                      else
                       { gint  taille = (yyvsp[-1].t_instruction)->condition->taille + (yyvsp[-1].t_instruction)->actions->taille_alors + (yyvsp[-1].t_instruction)->actions->taille_sinon + ((yyvsp[0].chaine) ? strlen((yyvsp[0].chaine)) : 0) + 256;
                         gchar *sinon = ((yyvsp[-1].t_instruction)->actions->sinon ? (yyvsp[-1].t_instruction)->actions->sinon : "/* no sinon action */");
                         (yyval.chaine) = New_chaine( taille );
                         g_snprintf( (yyval.chaine), taille,
                                     "/* -%06d------ une_instr BOOL--------*/\n"
                                     "vars->num_ligne = %d;\n"
                                     " { static gint prev_state = -1;\n"
                                     "   if (%s)\n"
                                     "    {\n"
                                     "%s\n"
                                     "      prev_state=1;\n"
                                     "    }\n"
                                     "   else\n"
                                     "    {\n"
                                     "      %s\n"
                                     "      prev_state=0;\n"
                                     "    }\n"
                                     " }\n"
                                     "%s",
                                     taille, (yyvsp[-1].t_instruction)->line_number, (yyvsp[-1].t_instruction)->condition->chaine, (yyvsp[-1].t_instruction)->actions->alors, sinon, ((yyvsp[0].chaine) ? (yyvsp[0].chaine) : "/**/") );
                       }
                    } else { (yyval.chaine)=NULL; }
                   Del_instruction((yyvsp[-1].t_instruction));
                   if ((yyvsp[0].chaine)) g_free((yyvsp[0].chaine));
                }}
#line 1894 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 24: /* listeInstr: un_switch listeInstr  */
#line 245 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].chaine))
                    { gint taille = strlen((yyvsp[-1].chaine)) + ((yyvsp[0].chaine) ? strlen((yyvsp[0].chaine)) : 0) + 128;
                      (yyval.chaine) = New_chaine( taille );
                      g_snprintf( (yyval.chaine), taille, "%s%s", (yyvsp[-1].chaine), ((yyvsp[0].chaine) ? (yyvsp[0].chaine) : "/* No listeInstr After switch */") );
                    } else { (yyval.chaine)=NULL; }
                   if ((yyvsp[-1].chaine)) g_free((yyvsp[-1].chaine));
                   if ((yyvsp[0].chaine)) g_free((yyvsp[0].chaine));
                }}
#line 1907 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 25: /* listeInstr: %empty  */
#line 253 "/home/sebastien/API/TraductionDLS/lignes.y"
                  {{ (yyval.chaine)=NULL; }}
#line 1913 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 26: /* une_instr: T_MOINS expr DONNE liste_action PVIRGULE  */
#line 257 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_instruction)=New_instruction ( scan_instance, (yyvsp[-3].t_condition), NULL, (yyvsp[-1].action) ); }}
#line 1919 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 27: /* une_instr: T_MOINS expr T_DIFFERE options DONNE liste_action PVIRGULE  */
#line 259 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_instruction)=New_instruction ( scan_instance, (yyvsp[-5].t_condition), (yyvsp[-3].gliste), (yyvsp[-1].action) ); }}
#line 1925 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 28: /* une_instr: T_MOINS expr DONNE T_ACCOUV listeInstr T_ACCFERM  */
#line 261 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].chaine))
                    { struct ACTION *action = New_action();
                      action->alors = (yyvsp[-1].chaine);
                      action->taille_alors = strlen((yyvsp[-1].chaine));
                      (yyval.t_instruction)=New_instruction ( scan_instance, (yyvsp[-4].t_condition), NULL, action );
                    } else (yyval.t_instruction)=NULL;
                }}
#line 1937 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 29: /* un_switch: T_SWITCH listeCase  */
#line 271 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ gint taille;
                   if ((yyvsp[0].chaine))
                    { taille = strlen((yyvsp[0].chaine)) + 100;
                      (yyval.chaine) = New_chaine( taille );
                      g_snprintf( (yyval.chaine), taille, "/* Ligne (CASE BEGIN)------------*/\n"
                                              "%s\n"
                                              "/* Ligne (CASE END)--------------*/\n\n",
                                              (yyvsp[0].chaine) );
                    } else { Emettre_erreur_new( scan_instance, "Switch list case is mandatory" ); (yyval.chaine)=NULL; }
                   if ((yyvsp[0].chaine)) g_free((yyvsp[0].chaine));
                }}
#line 1953 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 30: /* listeCase: T_PIPE une_instr listeCase  */
#line 284 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].t_instruction) && (yyvsp[-1].t_instruction)->condition && (yyvsp[-1].t_instruction)->condition->is_bool == FALSE)
                    { Emettre_erreur_new( scan_instance, "Boolean is left mandatory" ); (yyval.chaine)=NULL; }
                   else if ((yyvsp[-1].t_instruction))
                    { gchar *suite = ((yyvsp[0].chaine) ? (yyvsp[0].chaine) : "/* no suite */");
                      gint taille = (yyvsp[-1].t_instruction)->actions->taille_alors+(yyvsp[-1].t_instruction)->actions->taille_sinon+(yyvsp[-1].t_instruction)->condition->taille+256 + strlen(suite);
                      (yyval.chaine) = New_chaine( taille );
                      g_snprintf( (yyval.chaine), taille,
                                  "vars->num_ligne = %d; /* CASE INSIDE */\n"
                                  "if(%s)\n { %s }\nelse\n { %s\n%s }\n",
                                   (yyvsp[-1].t_instruction)->line_number, (yyvsp[-1].t_instruction)->condition->chaine, (yyvsp[-1].t_instruction)->actions->alors,
                                  ((yyvsp[-1].t_instruction)->actions->sinon ? (yyvsp[-1].t_instruction)->actions->sinon : "/* no action sinon */"), suite );
                    } else (yyval.chaine)=NULL;
                   Del_instruction((yyvsp[-1].t_instruction));
                   if ((yyvsp[0].chaine)) g_free((yyvsp[0].chaine));
                }}
#line 1973 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 31: /* listeCase: T_PIPE T_MOINS DONNE liste_action PVIRGULE  */
#line 300 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].action))
                    { gint taille = (yyvsp[-1].action)->taille_alors+48;
                      (yyval.chaine) = New_chaine( taille );
                      g_snprintf( (yyval.chaine), taille,
                                  " /* CASE INSIDE DEFAULT */\n"
                                  "  %s", (yyvsp[-1].action)->alors );
                    } else (yyval.chaine)=NULL;
                   Del_actions((yyvsp[-1].action));
                }}
#line 1987 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 32: /* expr: expr T_PLUS expr  */
#line 312 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].t_condition) && (yyvsp[0].t_condition))
                    { if ((yyvsp[-2].t_condition)->is_bool != (yyvsp[0].t_condition)->is_bool)
                       { Emettre_erreur_new( scan_instance, "Mixing Bool and Float is forbidden" ); (yyval.t_condition)=NULL; }
                      else
                       { (yyval.t_condition) = New_condition( (yyvsp[-2].t_condition)->is_bool, (yyvsp[-2].t_condition)->taille + (yyvsp[0].t_condition)->taille + 6 );
                         if ((yyval.t_condition) && (yyvsp[-2].t_condition)->is_bool)
                          { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(%s || %s)", (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                        else
                          { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(%s+%s)", (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2006 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 33: /* expr: expr T_MOINS expr  */
#line 327 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].t_condition) && (yyvsp[0].t_condition))
                    { if ((yyvsp[-2].t_condition)->is_bool == TRUE || (yyvsp[0].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Boolean not allowed within -" ); (yyval.t_condition)=NULL; }
                      else
                       { gint taille = (yyvsp[-2].t_condition)->taille + (yyvsp[0].t_condition)->taille + 3;
                         (yyval.t_condition) = New_condition( FALSE, taille );
                         if ((yyval.t_condition))
                          { g_snprintf( (yyval.t_condition)->chaine, taille, "(%s-%s)", (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2024 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 34: /* expr: expr ET expr  */
#line 341 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].t_condition) && (yyvsp[0].t_condition))
                    { if ((yyvsp[-2].t_condition)->is_bool == FALSE || (yyvsp[0].t_condition)->is_bool == FALSE)
                       { Emettre_erreur_new( scan_instance, "Boolean mandatory in AND" ); (yyval.t_condition)=NULL; }
                      else
                       { (yyval.t_condition) = New_condition( TRUE, (yyvsp[-2].t_condition)->taille + (yyvsp[0].t_condition)->taille + 6 );
                         if ((yyval.t_condition))
                          { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(%s && %s)", (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2041 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 35: /* expr: expr T_FOIS expr  */
#line 354 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].t_condition) && (yyvsp[0].t_condition))
                    { if ((yyvsp[-2].t_condition)->is_bool == TRUE || (yyvsp[0].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Float mandatory in *" ); (yyval.t_condition)=NULL; }
                      else
                       { (yyval.t_condition) = New_condition( FALSE, (yyvsp[-2].t_condition)->taille + (yyvsp[0].t_condition)->taille + 3 );
                         if ((yyval.t_condition))
                          { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(%s*%s)", (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2058 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 36: /* expr: expr BARRE expr  */
#line 367 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].t_condition) && (yyvsp[0].t_condition))
                    { if ((yyvsp[-2].t_condition)->is_bool == TRUE || (yyvsp[0].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Boolean not allowed within /" ); (yyval.t_condition)=NULL; }
                      else
                       { gint taille = (yyvsp[-2].t_condition)->taille + (yyvsp[0].t_condition)->taille + 45;
                         (yyval.t_condition) = New_condition( FALSE, taille );
                         if ((yyval.t_condition))
                          { g_snprintf( (yyval.t_condition)->chaine, taille, "(%s==0.0 ? 1.0 : ((gdouble)%s/%s))", (yyvsp[0].t_condition)->chaine, (yyvsp[-2].t_condition)->chaine, (yyvsp[0].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2076 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 37: /* expr: barre T_POUV expr T_PFERM  */
#line 381 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-3].val) && (yyvsp[-1].t_condition)->is_bool == FALSE) Emettre_erreur_new( scan_instance, "'!' allow only with boolean" );
                      else
                       { (yyval.t_condition) = New_condition( (yyvsp[-1].t_condition)->is_bool, (yyvsp[-1].t_condition)->taille+3 );
                         if ((yyvsp[-3].val)) { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "!(%s)", (yyvsp[-1].t_condition)->chaine ); }
                         else    { g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(%s)", (yyvsp[-1].t_condition)->chaine ); }
                       }
                    } else (yyval.t_condition)=NULL;
                   Del_condition((yyvsp[-1].t_condition));
                }}
#line 2091 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 38: /* expr: expr ordre expr  */
#line 392 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition_comparaison ( scan_instance, (yyvsp[-2].t_condition), (yyvsp[-1].val), (yyvsp[0].t_condition) );
                   Del_condition((yyvsp[-2].t_condition));
                   Del_condition((yyvsp[0].t_condition));
                }}
#line 2100 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 40: /* unite: barre un_alias liste_options  */
#line 400 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition_alias ( scan_instance, (yyvsp[-2].val), (yyvsp[-1].t_alias), (yyvsp[0].gliste) );
                   if((yyval.t_condition)==NULL) Liberer_options((yyvsp[0].gliste));
                }}
#line 2108 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 41: /* unite: T_VALF  */
#line 403 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.t_condition) = New_condition_valf ( (yyvsp[0].valf) );   }}
#line 2114 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 42: /* unite: ENTIER  */
#line 404 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.t_condition) = New_condition_entier ( (yyvsp[0].val) ); }}
#line 2120 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 43: /* unite: T_EXP T_POUV expr T_PFERM  */
#line 406 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in exp is forbidden" ); }
                      else (yyval.t_condition) = New_condition_exp ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2132 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 44: /* unite: T_ARCSIN T_POUV expr T_PFERM  */
#line 414 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arcsin is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_arcsin ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2144 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 45: /* unite: T_ARCTAN T_POUV expr T_PFERM  */
#line 422 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arctan is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_arctan ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2156 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 46: /* unite: T_ARCCOS T_POUV expr T_PFERM  */
#line 430 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arccos is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_arccos ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2168 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 47: /* unite: T_SIN T_POUV expr T_PFERM  */
#line 438 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arcsin is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_sin ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2180 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 48: /* unite: T_TAN T_POUV expr T_PFERM  */
#line 446 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arctan is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_tan ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2192 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 49: /* unite: T_COS T_POUV expr T_PFERM  */
#line 454 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition)=NULL;
                   if ((yyvsp[-1].t_condition))
                    { if ((yyvsp[-1].t_condition)->is_bool == TRUE)
                       { Emettre_erreur_new( scan_instance, "Using bool in arccos is forbidden" ); (yyval.t_condition)=NULL; }
                      else (yyval.t_condition) = New_condition_cos ( (yyvsp[-1].t_condition) );
                    }
                }}
#line 2204 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 50: /* unite: T_HEURE ordre ENTIER T_DPOINTS ENTIER  */
#line 462 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].val)>23) (yyvsp[-2].val)=23;
                   if ((yyvsp[-2].val)<0)  (yyvsp[-2].val)=0;
                   if ((yyvsp[0].val)>59) (yyvsp[0].val)=59;
                   if ((yyvsp[0].val)<0)  (yyvsp[0].val)=0;
                   (yyval.t_condition) = New_condition( TRUE, 32 );
                   if ((yyval.t_condition))
                    { switch ((yyvsp[-3].val))
                       { case T_EGAL     : g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Heure(%d,%d)", (yyvsp[-2].val), (yyvsp[0].val) );
                                           break;
                         case SUP_OU_EGAL: g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Heure_apres_egal(%d,%d)", (yyvsp[-2].val), (yyvsp[0].val) );
                                           break;
                         case INF_OU_EGAL: g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Heure_avant_egal(%d,%d)", (yyvsp[-2].val), (yyvsp[0].val) );
                                           break;
                         case SUP:         g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Heure_apres(%d,%d)", (yyvsp[-2].val), (yyvsp[0].val) );
                                           break;
                         case INF:         g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Heure_avant(%d,%d)", (yyvsp[-2].val), (yyvsp[0].val) );
                                           break;
                       }
                    }
                }}
#line 2229 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 51: /* unite: jour_semaine  */
#line 483 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition( TRUE, 18 );
                   if ((yyval.t_condition)) g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "Jour_semaine(%d)", (yyvsp[0].val) );
                }}
#line 2237 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 52: /* unite: T_START  */
#line 487 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition( TRUE, 20 );
                   if ((yyval.t_condition)) g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "(vars->resetted)" );
                }}
#line 2245 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 53: /* unite: T_TRUE  */
#line 491 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition( TRUE, 5 );
                   if ((yyval.t_condition)) g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "TRUE" );
                }}
#line 2253 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 54: /* unite: T_FALSE  */
#line 495 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_condition) = New_condition( TRUE, 5 );
                   if ((yyval.t_condition)) g_snprintf( (yyval.t_condition)->chaine, (yyval.t_condition)->taille, "FALSE" );
                }}
#line 2261 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 55: /* liste_action: liste_action VIRGULE une_action  */
#line 501 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ if ((yyvsp[-2].action) && (yyvsp[0].action))
                    { if( (yyvsp[-2].action)->is_float != (yyvsp[0].action)->is_float )
                       { Emettre_erreur_new( scan_instance, "Mix of bools and float forbidden in Action" );
                         (yyval.action) = NULL;
                       }
                      else
                       { (yyval.action) = New_action();
                         (yyval.action)->alors = g_strconcat ( ((yyvsp[-2].action)->alors ? (yyvsp[-2].action)->alors : ""), (yyvsp[0].action)->alors, NULL );
                         if ((yyval.action)->alors) (yyval.action)->taille_alors = strlen((yyval.action)->alors);
                         (yyval.action)->sinon = g_strconcat ( ((yyvsp[-2].action)->sinon ? (yyvsp[-2].action)->sinon : ""), (yyvsp[0].action)->sinon, NULL );
                         if ((yyval.action)->sinon) (yyval.action)->taille_sinon = strlen((yyval.action)->sinon);
                       }
                    } else (yyval.action)=NULL;
                   Del_actions ((yyvsp[-2].action));
                   Del_actions ((yyvsp[0].action));
                }}
#line 2282 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 56: /* liste_action: une_action  */
#line 518 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.action)=(yyvsp[0].action);
                   if ((yyval.action) && (yyval.action)->alors) (yyval.action)->taille_alors = strlen((yyval.action)->alors);
                   if ((yyval.action) && (yyval.action)->sinon) (yyval.action)->taille_sinon = strlen((yyval.action)->sinon);
                }}
#line 2291 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 57: /* une_action: T_NOP  */
#line 525 "/home/sebastien/API/TraductionDLS/lignes.y"
                  {{ (yyval.action)=New_action(); (yyval.action)->alors=g_strdup("/*NOP*/"); }}
#line 2297 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 58: /* une_action: T_PID liste_options  */
#line 527 "/home/sebastien/API/TraductionDLS/lignes.y"
                  {{ (yyval.action)=New_action_PID( scan_instance, (yyvsp[0].gliste) );
                     Liberer_options((yyvsp[0].gliste));
                  }}
#line 2305 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 59: /* une_action: barre un_alias liste_options  */
#line 531 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   alias = (yyvsp[-1].t_alias);                                       /* On recupere l'alias */
                   if (!alias) { (yyval.action) = NULL; }
                   else                                                           /* L'alias existe, vérifions ses parametres */
                    { alias->used_as_action = TRUE;
                      GList *options_g = g_list_copy( (yyvsp[0].gliste) );
                      GList *options_d = g_list_copy( alias->options );
                      GList *all_options = g_list_concat( options_g, options_d );       /* Concaténation des listes d'options */
                      if ((yyvsp[-2].val) && (alias->classe==T_TEMPO ||
                                 alias->classe==T_MSG ||
                                 alias->classe==T_BUS ||
                                 alias->classe==T_VISUEL ||
                                 alias->classe==T_WATCHDOG ||
                                 alias->classe==T_MONOSTABLE)
                         )
                       { Emettre_erreur_new( scan_instance, "'/%s' ne peut s'utiliser", alias->acronyme );
                         (yyval.action) = NULL;
                       }
                      else switch(alias->classe)
                       { case T_TEMPO         : (yyval.action)=New_action_tempo( scan_instance, alias ); break;
                         case T_MSG           : (yyval.action)=New_action_msg( scan_instance, alias );   break;
                         case T_BUS           : (yyval.action)=New_action_bus( scan_instance, alias, all_options );   break;
                         case T_BISTABLE      : (yyval.action)=New_action_bi( scan_instance, alias, (yyvsp[-2].val) ); break;
                         case T_MONOSTABLE    : (yyval.action)=New_action_mono( scan_instance, alias );   break;
                         case T_CPT_H         : (yyval.action)=New_action_cpt_h( scan_instance, alias, all_options );    break;
                         case T_CPT_IMP       : (yyval.action)=New_action_cpt_imp( scan_instance, alias, all_options );  break;
                         case T_VISUEL        : (yyval.action)=New_action_visuel( scan_instance, alias, (yyvsp[0].gliste) );            break;
                         case T_WATCHDOG      : (yyval.action)=New_action_WATCHDOG( scan_instance, alias, all_options ); break;
                         case T_REGISTRE      : (yyval.action)=New_action_REGISTRE( scan_instance, alias, all_options ); break;
                         case T_DIGITAL_OUTPUT: (yyval.action)=New_action_sortie( scan_instance, alias, (yyvsp[-2].val) );  break;
                         case T_ANALOG_OUTPUT : (yyval.action)=New_action_AO( scan_instance, alias, all_options ); break;
                         case T_DIGITAL_INPUT : (yyval.action)=New_action_DI( scan_instance, alias ); break;
                         default: { Emettre_erreur_new( scan_instance, "'%s:%s' syntax error", alias->tech_id, alias->acronyme );
                                    (yyval.action)=NULL;
                                  }
                       }
                      g_list_free(all_options);
                    }
                   Liberer_options((yyvsp[0].gliste));                                                    /* On libére les options "locales" */
                }}
#line 2350 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 60: /* barre: BARRE  */
#line 573 "/home/sebastien/API/TraductionDLS/lignes.y"
                      {{ (yyval.val)=1; }}
#line 2356 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 61: /* barre: %empty  */
#line 574 "/home/sebastien/API/TraductionDLS/lignes.y"
                      {{ (yyval.val)=0; }}
#line 2362 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 62: /* jour_semaine: LUNDI  */
#line 576 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=1; }}
#line 2368 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 63: /* jour_semaine: MARDI  */
#line 577 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=2; }}
#line 2374 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 64: /* jour_semaine: MERCREDI  */
#line 578 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=3; }}
#line 2380 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 65: /* jour_semaine: JEUDI  */
#line 579 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=4; }}
#line 2386 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 66: /* jour_semaine: VENDREDI  */
#line 580 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=5; }}
#line 2392 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 67: /* jour_semaine: SAMEDI  */
#line 581 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=6; }}
#line 2398 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 68: /* jour_semaine: DIMANCHE  */
#line 582 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.val)=0; }}
#line 2404 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 74: /* liste_options: T_POUV options T_PFERM  */
#line 587 "/home/sebastien/API/TraductionDLS/lignes.y"
                                         {{ (yyval.gliste) = (yyvsp[-1].gliste);   }}
#line 2410 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 75: /* liste_options: T_POUV T_PFERM  */
#line 588 "/home/sebastien/API/TraductionDLS/lignes.y"
                                         {{ (yyval.gliste) = NULL; }}
#line 2416 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 76: /* liste_options: %empty  */
#line 589 "/home/sebastien/API/TraductionDLS/lignes.y"
                                         {{ (yyval.gliste) = NULL; }}
#line 2422 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 77: /* options: options VIRGULE une_option  */
#line 593 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.gliste) = g_list_append( (yyvsp[-2].gliste), (yyvsp[0].option) );   }}
#line 2428 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 78: /* options: une_option  */
#line 594 "/home/sebastien/API/TraductionDLS/lignes.y"
                             {{ (yyval.gliste) = g_list_append( NULL, (yyvsp[0].option) ); }}
#line 2434 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 79: /* une_option: T_CONSIGNE T_EGAL ENTIER  */
#line 598 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2444 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 80: /* une_option: T_GROUPE T_EGAL ENTIER  */
#line 604 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2454 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 81: /* une_option: T_LIBELLE T_EGAL T_CHAINE  */
#line 610 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2464 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 82: /* une_option: T_DEFAUT T_EGAL T_CHAINE  */
#line 616 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2474 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 83: /* une_option: T_DEFAUT T_EGAL ENTIER  */
#line 622 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2484 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 84: /* une_option: T_UNITE T_EGAL T_CHAINE  */
#line 628 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2494 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 85: /* une_option: T_EDGE_UP  */
#line 634 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2504 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 86: /* une_option: T_EDGE_DOWN  */
#line 640 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2514 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 87: /* une_option: T_IN_RANGE  */
#line 646 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2524 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 88: /* une_option: T_HOST T_EGAL T_CHAINE  */
#line 653 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2534 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 89: /* une_option: T_TECH_ID T_EGAL T_CHAINE  */
#line 659 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2544 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 90: /* une_option: T_COMMANDE T_EGAL T_CHAINE  */
#line 665 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2554 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 91: /* une_option: T_MODE T_EGAL T_CHAINE  */
#line 671 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2564 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 92: /* une_option: T_BADGE T_EGAL T_CHAINE  */
#line 677 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2574 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 93: /* une_option: T_MAP_SMS T_EGAL T_CHAINE  */
#line 683 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2584 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 94: /* une_option: T_COLOR T_EGAL couleur  */
#line 689 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = g_strdup((yyvsp[0].chaine));
                }}
#line 2594 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 95: /* une_option: T_COLOR T_EGAL T_CHAINE  */
#line 695 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   if (!strcasecmp ( (yyvsp[0].chaine), "grey" )) (yyval.option)->chaine = g_strdup ( "gray" );
                                              else (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2605 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 96: /* une_option: T_FORME T_EGAL T_CHAINE  */
#line 702 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_CHAINE;
                   (yyval.option)->chaine = (yyvsp[0].chaine);
                }}
#line 2615 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 97: /* une_option: CLIGNO  */
#line 708 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2625 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 98: /* une_option: CLIGNO T_EGAL ENTIER  */
#line 714 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2635 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 99: /* une_option: T_DEBUG  */
#line 720 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = T_DEBUG;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2645 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 100: /* une_option: T_DISABLE  */
#line 726 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = T_DISABLE;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2655 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 101: /* une_option: T_RESET  */
#line 732 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2665 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 102: /* une_option: T_RW  */
#line 738 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2675 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 103: /* une_option: T_RESET T_EGAL ENTIER  */
#line 744 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2685 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 104: /* une_option: T_MULTI T_EGAL T_VALF  */
#line 750 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2695 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 105: /* une_option: T_TYPE T_EGAL type_msg  */
#line 756 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2705 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 106: /* une_option: T_DAA T_EGAL ENTIER  */
#line 762 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2715 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 107: /* une_option: T_DMINA T_EGAL ENTIER  */
#line 768 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2725 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 108: /* une_option: T_DMAXA T_EGAL ENTIER  */
#line 774 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2735 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 109: /* une_option: T_DAD T_EGAL ENTIER  */
#line 780 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2745 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 110: /* une_option: T_RANDOM T_EGAL ENTIER  */
#line 786 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2755 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 111: /* une_option: T_MIN T_EGAL ENTIER  */
#line 792 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2765 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 112: /* une_option: T_MIN T_EGAL T_VALF  */
#line 798 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2775 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 113: /* une_option: T_MAX T_EGAL ENTIER  */
#line 804 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2785 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 114: /* une_option: T_MAX T_EGAL T_VALF  */
#line 810 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2795 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 115: /* une_option: T_SEUIL_NTB T_EGAL ENTIER  */
#line 816 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2805 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 116: /* une_option: T_SEUIL_NTB T_EGAL T_VALF  */
#line 822 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2815 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 117: /* une_option: T_SEUIL_NB T_EGAL ENTIER  */
#line 828 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2825 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 118: /* une_option: T_SEUIL_NB T_EGAL T_VALF  */
#line 834 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2835 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 119: /* une_option: T_SEUIL_NH T_EGAL ENTIER  */
#line 840 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2845 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 120: /* une_option: T_SEUIL_NH T_EGAL T_VALF  */
#line 846 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2855 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 121: /* une_option: T_SEUIL_NTH T_EGAL ENTIER  */
#line 852 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = 1.0*(yyvsp[0].val);
                }}
#line 2865 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 122: /* une_option: T_SEUIL_NTH T_EGAL T_VALF  */
#line 858 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = T_VALF;
                   (yyval.option)->val_as_double = (yyvsp[0].valf);
                }}
#line 2875 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 123: /* une_option: T_DECIMAL T_EGAL ENTIER  */
#line 864 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2885 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 124: /* une_option: T_NOSHOW  */
#line 870 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = 1;
                }}
#line 2895 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 125: /* une_option: T_FREEZE T_EGAL ENTIER  */
#line 876 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 2905 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 126: /* une_option: T_CONSIGNE T_EGAL un_alias  */
#line 882 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2915 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 127: /* une_option: T_INPUT T_EGAL un_alias  */
#line 888 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2925 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 128: /* une_option: T_OUTPUT T_EGAL un_alias  */
#line 894 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2935 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 129: /* une_option: T_KP T_EGAL un_alias  */
#line 900 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2945 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 130: /* une_option: T_KI T_EGAL un_alias  */
#line 906 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2955 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 131: /* une_option: T_KD T_EGAL un_alias  */
#line 912 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2965 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 132: /* une_option: T_MIN T_EGAL un_alias  */
#line 918 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2975 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 133: /* une_option: T_MAX T_EGAL un_alias  */
#line 924 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ID;
                   (yyval.option)->val_as_alias = (yyvsp[0].t_alias);
                }}
#line 2985 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 134: /* une_option: T_NOTIF_SMS  */
#line 930 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = T_YES;
                }}
#line 2995 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 135: /* une_option: T_NOTIF_SMS T_EGAL type_notif_sms  */
#line 936 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 3005 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 136: /* une_option: T_NOTIF_CHAT  */
#line 942 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[0].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = T_YES;
                }}
#line 3015 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 137: /* une_option: T_NOTIF_CHAT T_EGAL type_notif_chat  */
#line 948 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.option)=New_option();
                   (yyval.option)->token = (yyvsp[-2].val);
                   (yyval.option)->token_classe = ENTIER;
                   (yyval.option)->val_as_int = (yyvsp[0].val);
                }}
#line 3025 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 138: /* couleur: T_ROUGE  */
#line 955 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="red";       }}
#line 3031 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 139: /* couleur: T_VERT  */
#line 956 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="green";     }}
#line 3037 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 140: /* couleur: T_BLEU  */
#line 957 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="blue";      }}
#line 3043 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 141: /* couleur: T_JAUNE  */
#line 958 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="yellow";    }}
#line 3049 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 142: /* couleur: T_NOIR  */
#line 959 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="black";     }}
#line 3055 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 143: /* couleur: T_BLANC  */
#line 960 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="white";     }}
#line 3061 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 144: /* couleur: T_GRIS  */
#line 961 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="gray";      }}
#line 3067 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 145: /* couleur: T_ORANGE  */
#line 962 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="orange";    }}
#line 3073 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 146: /* couleur: T_KAKI  */
#line 963 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="darkgreen"; }}
#line 3079 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 147: /* couleur: T_CYAN  */
#line 964 "/home/sebastien/API/TraductionDLS/lignes.y"
                           {{ (yyval.chaine)="lightblue"; }}
#line 3085 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 148: /* type_msg: T_ETAT  */
#line 966 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_ETAT;        }}
#line 3091 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 149: /* type_msg: T_NOTIF  */
#line 967 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_NOTIF;       }}
#line 3097 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 150: /* type_msg: T_DEFAUT  */
#line 968 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_DEFAUT;      }}
#line 3103 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 151: /* type_msg: T_ALARME  */
#line 969 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_ALARME;      }}
#line 3109 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 152: /* type_msg: T_VEILLE  */
#line 970 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_VEILLE;      }}
#line 3115 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 153: /* type_msg: T_ALERTE  */
#line 971 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_ALERTE;      }}
#line 3121 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 154: /* type_msg: T_DANGER  */
#line 972 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_DANGER;      }}
#line 3127 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 155: /* type_msg: T_DERANGEMENT  */
#line 973 "/home/sebastien/API/TraductionDLS/lignes.y"
                                {{ (yyval.val)=MSG_DERANGEMENT; }}
#line 3133 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 161: /* un_alias: ID  */
#line 980 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_alias) = Get_local_alias ( scan_instance, NULL, (yyvsp[0].chaine) );
                   if (!(yyval.t_alias))
                    { (yyval.t_alias) = New_external_alias( scan_instance, NULL, (yyvsp[0].chaine), NULL ); }    /* Si dependance externe, on va chercher */
                   if (!(yyval.t_alias))
                    { (yyval.t_alias) = New_external_alias( scan_instance, "SYS", (yyvsp[0].chaine), NULL ); }   /* Si dependance externe, on va chercher */
                   if (!(yyval.t_alias))
                    { Emettre_erreur_new( scan_instance, "'%s' is not defined", (yyvsp[0].chaine) ); }
                   g_free((yyvsp[0].chaine));
                }}
#line 3147 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;

  case 162: /* un_alias: ID T_DPOINTS ID  */
#line 990 "/home/sebastien/API/TraductionDLS/lignes.y"
                {{ (yyval.t_alias) = Get_local_alias ( scan_instance, (yyvsp[-2].chaine), (yyvsp[0].chaine) );
                   if (!(yyval.t_alias))
                    { (yyval.t_alias) = New_external_alias( scan_instance, (yyvsp[-2].chaine), (yyvsp[0].chaine), NULL ); }      /* Si dependance externe, on va chercher */
                   if (!(yyval.t_alias))
                    { Emettre_erreur_new( scan_instance, "'%s:%s' is not defined", (yyvsp[-2].chaine), (yyvsp[0].chaine) ); }
                   g_free((yyvsp[-2].chaine));
                   g_free((yyvsp[0].chaine));
                }}
#line 3160 "/home/sebastien/API/build/TraductionDLS/lignes.c"
    break;


#line 3164 "/home/sebastien/API/build/TraductionDLS/lignes.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (scan_instance, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scan_instance);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scan_instance);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scan_instance, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scan_instance);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scan_instance);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 998 "/home/sebastien/API/TraductionDLS/lignes.y"


/*----------------------------------------------------------------------------------------------------------------------------*/
