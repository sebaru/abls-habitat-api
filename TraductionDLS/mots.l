/******************************************************************************************************************************/
/* TraductionDLS/mots.l        Définitions des mots DLS                                                                       */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                    jeu. 24 juin 2010 19:37:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mots.l
 * This file is part of Abls-Habitat
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


%{
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "Http.h"
#include "Dls_trad.h"
%}

%option reentrant bison-bridge
%option prefix = "DlsScanner_"
/* On n'utilise pas la fonction yywrap associée */
%option 8bit noyywrap nounput noinput
/* On garde le numéro de ligne lors du scan des tokens */
%option yylineno
%option debug
%option header-file = "mots.h"
%option outfile = "mots.c"

/* Expressions rationnelles de detection */
digit         [-]?[0-9]+
digitf        [-]?[0-9]+[\.[0-9]+]?
id            [a-zA-Z][a-zA-Z_0-9]+
chaine        [^"]*
blanc         [\t ]+
newline       [\n\r]+

 /* Noms des environnements */
%x code_en_c
%x chaine_directe
%x comment

%%
{blanc}
{newline}

 /******************************************** Methodes ***************************************************/
"_EA"                      { yylval->val = T_ANALOG_INPUT;  return T_ANALOG_INPUT;  }
"_AI"                      { yylval->val = T_ANALOG_INPUT;  return T_ANALOG_INPUT;  }
"_AO"                      { yylval->val = T_ANALOG_OUTPUT; return T_ANALOG_OUTPUT; }
"_E"                       { yylval->val = T_DIGITAL_INPUT; return T_DIGITAL_INPUT; }
"_DI"                      { yylval->val = T_DIGITAL_INPUT; return T_DIGITAL_INPUT; }
"_A"                       { yylval->val = T_DIGITAL_OUTPUT;return T_DIGITAL_OUTPUT;}
"_DO"                      { yylval->val = T_DIGITAL_OUTPUT;return T_DIGITAL_OUTPUT;}
"_I"                       { yylval->val = T_VISUEL;        return T_VISUEL;        }
"_VISUEL"                  { yylval->val = T_VISUEL;        return T_VISUEL;        }
"_B"                       { yylval->val = T_BISTABLE;      return T_BISTABLE;      }
"_M"                       { yylval->val = T_MONOSTABLE;    return T_MONOSTABLE;    }
"_T"                       { yylval->val = T_TEMPO;         return T_TEMPO;         }
"_TEMPO"                   { yylval->val = T_TEMPO;         return T_TEMPO;         }
"_R"                       { yylval->val = T_REGISTRE;      return T_REGISTRE;      }
"_REGISTRE"                { yylval->val = T_REGISTRE;      return T_REGISTRE;      }
"_CH"                      { yylval->val = T_CPT_H;         return T_CPT_H;         }
"_CI"                      { yylval->val = T_CPT_IMP;       return T_CPT_IMP;       }
"_HORLOGE"                 { yylval->val = T_HORLOGE;       return T_HORLOGE;       }
"_WATCHDOG"                { yylval->val = T_WATCHDOG;      return T_WATCHDOG;      }
"_MSG"                     { yylval->val = T_MSG;           return T_MSG;           }
"_START"                   { yylval->val = T_START;         return T_START;         }
"_TRUE"                    { yylval->val = T_TRUE;          return T_TRUE;          }
"_FALSE"                   { yylval->val = T_FALSE;         return T_FALSE;         }
"_BUS"                     { yylval->val = T_BUS;           return T_BUS;           }
"_PID"                     { yylval->val = T_PID;           return T_PID;           }
"_ARCSIN"                  { yylval->val = T_ARCSIN;        return T_ARCSIN;        }
"_ARCTAN"                  { yylval->val = T_ARCTAN;        return T_ARCTAN;        }
"_ARCCOS"                  { yylval->val = T_ARCCOS;        return T_ARCCOS;        }
"_SIN"                     { yylval->val = T_SIN;           return T_SIN;           }
"_TAN"                     { yylval->val = T_TAN;           return T_TAN;           }
"_COS"                     { yylval->val = T_COS;           return T_COS;           }
"_EXP"                     { yylval->val = T_EXP;           return T_EXP;           }
"_NOP"                     { yylval->val = T_NOP;           return T_NOP;           }
 /***************************************** Methodes speciales ********************************************/
"_HEURE"                   { yylval->val = T_HEURE;     return T_HEURE;     }
"_APRES"                   { yylval->val = APRES;       return APRES;       }
"_AVANT"                   { yylval->val = AVANT;       return AVANT;       }
"_LUNDI"                   { yylval->val = LUNDI;       return LUNDI;       }
"_MARDI"                   { yylval->val = MARDI;       return MARDI;       }
"_MERCREDI"                { yylval->val = MERCREDI;    return MERCREDI;    }
"_JEUDI"                   { yylval->val = JEUDI;       return JEUDI;       }
"_VENDREDI"                { yylval->val = VENDREDI;    return VENDREDI;    }
"_SAMEDI"                  { yylval->val = SAMEDI;      return SAMEDI;      }
"_DIMANCHE"                { yylval->val = DIMANCHE;    return DIMANCHE;    }
 /************************************************ Couleurs ***********************************************/
"rouge"                    { yylval->val = T_ROUGE;     return T_ROUGE;     }
"vert"                     { yylval->val = T_VERT;      return T_VERT;      }
"bleu"                     { yylval->val = T_BLEU;      return T_BLEU;      }
"cyan"                     { yylval->val = T_CYAN;      return T_CYAN;      }
"jaune"                    { yylval->val = T_JAUNE;     return T_JAUNE;     }
"noir"                     { yylval->val = T_NOIR;      return T_NOIR;      }
"orange"                   { yylval->val = T_ORANGE;    return T_ORANGE;    }
"gris"                     { yylval->val = T_GRIS;      return T_GRIS;      }
"blanc"                    { yylval->val = T_BLANC;     return T_BLANC;     }
"kaki"                     { yylval->val = T_KAKI;      return T_KAKI;      }
 /************************************************* Options ************************************************/
"debug"                    { yylval->val = T_DEBUG;     return T_DEBUG;     }
"disable"                  { yylval->val = T_DISABLE;   return T_DISABLE;   }
"mode"                     { yylval->val = T_MODE;      return T_MODE;      }
"reset"                    { yylval->val = T_RESET;     return T_RESET;     }
"multi"                    { yylval->val = T_MULTI;     return T_MULTI;     }
"ratio"                    { yylval->val = T_RATIO;     return T_RATIO;     }
"color"                    { yylval->val = T_COLOR;     return T_COLOR;     }
"cligno"                   { yylval->val = CLIGNO;      return CLIGNO;      }
"in_range"                 { yylval->val = T_IN_RANGE;  return T_IN_RANGE;  }
"edge_up"                  { yylval->val = T_EDGE_UP;   return T_EDGE_UP;   }
"edge_down"                { yylval->val = T_EDGE_DOWN; return T_EDGE_DOWN; }
"libelle"                  { yylval->val = T_LIBELLE;   return T_LIBELLE;   }
"unite"                    { yylval->val = T_UNITE;     return T_UNITE;     }
"groupe"                   { yylval->val = T_GROUPE;    return T_GROUPE;    }
"random"                   { yylval->val = T_RANDOM;    return T_RANDOM;    }
"daa"                      { yylval->val = T_DAA;       return T_DAA;       }
"dma"                      { yylval->val = T_DMINA;     return T_DMINA;     }
"dMa"                      { yylval->val = T_DMAXA;     return T_DMAXA;     }
"dad"                      { yylval->val = T_DAD;       return T_DAD;       }
"consigne"                 { yylval->val = T_CONSIGNE;  return T_CONSIGNE;  }
"type"                     { yylval->val = T_TYPE;      return T_TYPE;      }
"etat"                     { yylval->val = T_ETAT;      return T_ETAT;      }
"notification"             { yylval->val = T_NOTIF;     return T_NOTIF;     }
"notif_gsm"                { yylval->val = T_NOTIF_GSM; return T_NOTIF_GSM; }
"notif_chat"               { yylval->val = T_NOTIF_CHAT;return T_NOTIF_CHAT;}
"yes"                      { yylval->val = T_YES;       return T_YES;       }
"no"                       { yylval->val = T_NO;        return T_NO;        }
"ovh_only"                 { yylval->val = T_OVH_ONLY;  return T_OVH_ONLY;  }
"defaut"                   { yylval->val = T_DEFAUT;    return T_DEFAUT;    }
"alarme"                   { yylval->val = T_ALARME;    return T_ALARME;    }
"veille"                   { yylval->val = T_VEILLE;    return T_VEILLE;    }
"alerte"                   { yylval->val = T_ALERTE;    return T_ALERTE;    }
"derangement"              { yylval->val = T_DERANGEMENT;return T_DERANGEMENT;}
"danger"                   { yylval->val = T_DANGER;    return T_DANGER;    }
"host"                     { yylval->val = T_HOST;      return T_HOST;      }
"tech_id"                  { yylval->val = T_TECH_ID;   return T_TECH_ID;   }
"tag"                      { yylval->val = T_TAG;       return T_TAG;       }
"command"                  { yylval->val = T_COMMAND;   return T_COMMAND;   }
"forme"                    { yylval->val = T_FORME;     return T_FORME;     }
"input"                    { yylval->val = T_INPUT;     return T_INPUT;     }
"kp"                       { yylval->val = T_KP;        return T_KP;        }
"ki"                       { yylval->val = T_KI;        return T_KI;        }
"kd"                       { yylval->val = T_KD;        return T_KD;        }
"min"                      { yylval->val = T_MIN;       return T_MIN;       }
"max"                      { yylval->val = T_MAX;       return T_MAX;       }
"seuil_ntb"                { yylval->val = T_SEUIL_NTB; return T_SEUIL_NTB; }
"seuil_nb"                 { yylval->val = T_SEUIL_NB;  return T_SEUIL_NB;  }
"seuil_nh"                 { yylval->val = T_SEUIL_NH;  return T_SEUIL_NH;  }
"seuil_nth"                { yylval->val = T_SEUIL_NTH; return T_SEUIL_NTH; }
"decimal"                  { yylval->val = T_DECIMAL;   return T_DECIMAL;   }
"noshow"                   { yylval->val = T_NOSHOW;    return T_NOSHOW;    }
 /*********************************************** Le reste ************************************************/
"#param"                   { yylval->val = T_PARAM;     return T_PARAM;     }
"#define"                  { yylval->val = T_DEFINE;    return T_DEFINE;    }
"#link"                    { yylval->val = T_LINK;      return T_LINK;      }
"switch"                   { yylval->val = T_SWITCH;    return T_SWITCH;    }
"."                        { yylval->val = ET;          return ET;          }
"+"                        { yylval->val = T_PLUS;      return T_PLUS;      }
"/"                        { yylval->val = BARRE;       return BARRE;       }
"*"                        { yylval->val = T_FOIS;      return T_FOIS;      }
"|"                        { yylval->val = T_PIPE;      return T_PIPE;      }
"-"                        { yylval->val = T_MOINS;     return T_MOINS;     }
";"                        { yylval->val = PVIRGULE;    return PVIRGULE;    }
","                        { yylval->val = VIRGULE;     return VIRGULE;     }
":"                        { yylval->val = T_DPOINTS;   return T_DPOINTS;   }
"("                        { yylval->val = T_POUV;      return T_POUV;      }
")"                        { yylval->val = T_PFERM;     return T_PFERM;     }
"{"                        { yylval->val = T_ACCOUV;    return T_ACCOUV;    }
"}"                        { yylval->val = T_ACCFERM;   return T_ACCFERM;   }
"="                        { yylval->val = T_EGAL;      return T_EGAL;      }
"<->"                      { yylval->val = EQUIV;       return EQUIV;       }
"->"                       { yylval->val = DONNE;       return DONNE;       }
"--"                       { yylval->val = T_DIFFERE;   return T_DIFFERE;   }
"<"                        { yylval->val = INF;         return INF;         }
">"                        { yylval->val = SUP;         return SUP;         }
"<="                       { yylval->val = INF_OU_EGAL; return INF_OU_EGAL; }
">="                       { yylval->val = SUP_OU_EGAL; return SUP_OU_EGAL; }

"/*"                       { BEGIN(comment); }
<comment>"*/"              { BEGIN(INITIAL); }
<comment>\n                {  }
<comment>.                 ;

"\""                       { BEGIN(chaine_directe); }
<chaine_directe>"\""       { yytext[yyleng-1]=0; yylval->chaine = (char *)strdup(yytext); BEGIN(INITIAL); return T_CHAINE; }
<chaine_directe>.          { yymore(); }

{id}                       { yylval->chaine = (char *)strdup(yytext); return ID;       }
{digit}                    { yylval->val    = atoi(yytext);           return ENTIER;   }
{digitf}                   { yylval->valf   = atof(yytext);           return T_VALF;   }

.                          { yylval->val = T_ERROR; return T_ERROR; }
 /*------------------------------------------------------------------------------------------------------*/
