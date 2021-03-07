/* See LICENSE file for copyright and license details. */
#include "keepfloatingposition.c"
#include "selfrestart.c"
/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int gappx     = 11;        /* gaps between windows */
static const unsigned int snap      = 20;       /* snap pixel */
static const unsigned int systraypinning = 1;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 1;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     	/* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "Fantasque Sans Mono:size=8" };
static const char dmenufont[]       = "Fantasque Sans Mono:size=8";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_brown[]       = "#523333";
static const char col_red[]         = "#fa1616";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_brown,  col_red  },
};

/* Autostart*/
static const char *const autostart[] = {
	"compton", NULL,
	"slstatus", NULL,
	"nm-applet", NULL,
	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  "Firefox",       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.65; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "T",      tile },    /* first entry is default */
	{ "F",      NULL },    /* no layout function means floating behavior */
	{ "M",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* extra keyboard keys */
#define XF86AudioLowerVolume	0x1008ff11
#define XF86AudioMute           0x1008ff12
#define XF86AudioRaiseVolume	0x1008ff13
#define XF86AudioPlay           0x1008ff14
#define XF86AudioStop    	0x1008ff15
#define XF86AudioPrev    	0x1008ff16
#define XF86AudioNext           0x1008ff17
#define XF86HomePage	        0x1008ff18
#define XF86Mail		0x1008ff19
#define XF86Tools	        0x1008ff81
#define XF86Calculator          0x1008ff1d
#define XF86Explorer            0x1008ff5d
#define XF_Print            	0xff61

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_brown, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };

/* My Commands */
static const char *fm[]  = { "thunar", NULL };
static const char *calc[]  = { "galculator", NULL };
static const char *vlc[]  = { "vlc","Music","-d", NULL };
static const char *mail[]  = { "thunderbird", NULL };
static const char *firefox[]  = { "firefox", NULL };
static const char *chrome[]  = { "chromium", NULL };
static const char *surf[]  = { "surf", NULL };
static const char *xran[]  = { "arandr", NULL };
static const char *appfinder[]  = { "xfce4-appfinder", NULL };
static const char *cam[]  = { "ffplay", "/dev/video0", NULL };
static const char *code[]  = { "code-oss", NULL };
static const char *screenshoot[]  = { "xfce-screenshooter", NULL };
static const char *downloader[]  = { "clipgrab", NULL };

/*audio controls*/
static const char *upvol[]   = { "amixer", "sset", "Master",    "5%+",     NULL };
static const char *downvol[] = { "amixer", "sset", "Master",    "5%-",     NULL };
static const char *mutevol[] = { "amixer", "-D",   "pulse",     "set",    "Master", "+1", "toggle", NULL };
static const char *playpause[] = { "playerctl", "play-pause", NULL };
static const char *stop[] = { "playerctl", "stop", NULL };
static const char *next[] = { "playerctl", "next", NULL };
static const char *prev[] = { "playerctl", "previous", NULL };

/* backlight controls (xorg-xbacklight) */
static const char *lightinc[]   = { "xbacklight", "-inc", "5",     NULL };
static const char *lightdec[]   = { "xbacklight", "-dec", "5",     NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ Mod4Mask|ShiftMask,           XK_Return, spawn,          {.v = code } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
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
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_r,      self_restart,   {0} },
        { 0,/*audio: up*/          	XF86AudioRaiseVolume,	spawn,	{.v = upvol   } },
        { 0,/*audio: down*/        	XF86AudioLowerVolume,	spawn,  {.v = downvol } },
        { 0,/*audio: mute Toggle*/ 	XF86AudioMute,       	spawn,  {.v = mutevol } },

        { MODKEY,/*backlight: increase*/	XF86AudioRaiseVolume,	spawn,	{.v = lightinc   } },
        { MODKEY,/*backlight: decrease*/	XF86AudioLowerVolume,	spawn,  {.v = lightdec } },

	{ 0,/*open file manager*/  		XF86Explorer,		spawn,  {.v = fm   } },
	{ 0,/*open mail app*/      		XF86Mail,		spawn,  {.v = mail } },

	{ 0,/*launch browser*/ 	   		XF86HomePage,		spawn,  {.v = firefox } },
	{ Mod4Mask,/*launch browser*/ 	   	XF86HomePage,		spawn,  {.v = surf } },
	{ MODKEY,/*launch browser*/ 	   	XF86HomePage,		spawn,  {.v = chrome } },

	{ 0,/* lauch calculator*/    		XF86Calculator,		spawn,  {.v = calc } },
	{ 0,/* lauch vlc*/    			XF86Tools,		spawn,  {.v = vlc } },

	{ 0,/*play pause*/ 	   		XF86AudioPlay,		spawn,  {.v = playpause } },
	{ 0,/*stop*/ 	   			XF86AudioStop,		spawn,  {.v = stop } },
	{ 0,/*next*/ 	   			XF86AudioNext,		spawn,  {.v = next } },
	{ 0,/*previous*/ 	   		XF86AudioPrev,		spawn,  {.v = prev } },

	{ Mod4Mask,/*screen setting*/		XF86Explorer,		spawn,  {.v = xran } },
	{ Mod4Mask,/*cam display*/ 		XF86AudioStop,		spawn,  {.v = cam } },
	{ Mod4Mask,/*downloader*/ 	   	XF86AudioPlay,		spawn,  {.v = downloader } },
	{ Mod4Mask,/*appfinder*/ 		XK_p,			spawn,  {.v = appfinder } },
	{ MODKEY,/*screenshooter*/ 			XF86Explorer,		spawn,  {.v = screenshoot } },
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
