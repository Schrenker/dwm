#include <X11/XF86keysym.h>
#include "gaplessgrid.c"

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 1;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display */
static const int showsystray        = 1;     /* 0 means no systray */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int horizpadbar        = 2;        /* horizontal padding for statusbar */
static const int vertpadbar         = 8;        /* vertical padding for statusbar */
static const unsigned int gappx     = 10;        /* gaps between windows */
static const char *fonts[]          = { "Iosevka Nerd Font:size=11" };
static const char dmenufont[]       = "Iosevka Nerd Font:size=11";
static const char mikado[]          = "#FEC901";
static const char dark_mikado[]     = "#FF9C0C";
static const char black[] 	    = "#333333";
static const char white[] 	    = "#FFFFFF";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, black },
	[SchemeSel]  = { col_gray4, col_cyan,  mikado  },
	[SchemeStatus]  = { white, black,  "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  = { black, mikado,  "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
	[SchemeTagsNorm]  = { mikado, black,  "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeInfoSel]  = { black, mikado,  "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
	[SchemeInfoNorm]  = { mikado, black,  "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
};
static const char *upvol[]   = { "/usr/bin/pactl", "set-sink-volume", "0", "+5%",     NULL };
static const char *downvol[] = { "/usr/bin/pactl", "set-sink-volume", "0", "-5%",     NULL };
static const char *mutevol[] = { "/usr/bin/pactl", "set-sink-mute",   "0", "toggle",  NULL };

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Brave",   NULL,     NULL,           0,         0,          0,          -1,        -1 },
	{ "Discord", NULL,     NULL,           1 << 8,         0,          0,          -1,        -1 },
	{ "st-256color",      NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
	{ "###",      gaplessgrid },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-F", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL};
static const char *termcmd[]  = { "st", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_o,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[5] } },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0  } },
	{ 0,                       XF86XK_AudioLowerVolume, spawn, {.v = downvol } },
	{ 0,                       XF86XK_AudioMute, spawn, {.v = mutevol } },
	{ 0,                       XF86XK_AudioRaiseVolume, spawn, {.v = upvol   } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
        { MODKEY|ShiftMask,             XK_r,      quit,           {1} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

