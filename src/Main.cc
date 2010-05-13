/* vim:set et sts=4: */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <ibus.h>
#include <stdlib.h>
#include <locale.h>
#include "Engine.h"
#include "Pointer.h"
#include "Bus.h"
#include "Config.h"

using namespace PY;

#define N_(text) text

static Pointer<IBusFactory> factory;

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
    { "ibus",    'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};


static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    g_debug ("bus disconnected");
    ibus_quit ();
}


static void
start_component (void)
{
    Pointer<IBusComponent> component;

    ibus_init ();
    Bus bus;

    if (!bus.isConnected ()) {
        g_warning ("Can not connect to ibus");
        exit (0);
    }

    Config config (bus);

    g_signal_connect ((IBusBus *)bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

    component = ibus_component_new ("org.freedesktop.IBus.Pinyin",
                                    N_("Pinyin input method"),
                                    VERSION,
                                    "GPL",
                                    "Peng Huang <shawn.p.huang@gmail.com>",
                                    "http://code.google.com/p/ibus/",
                                    "",
                                    "ibus-pinyin");
    ibus_component_add_engine (component,
                               ibus_engine_desc_new ("pinyin-debug",
                                                     N_("Pinyin (debug)"),
                                                     N_("Pinyin input method (debug)"),
                                                     "zh_CN",
                                                     "GPL",
                                                     "Peng Huang <shawn.p.huang@gmail.com>",
                                                     PKGDATADIR"/icons/ibus-pinyin.svg",
                                                     "us"));
    ibus_component_add_engine (component,
                               ibus_engine_desc_new ("bopomofo-debug",
                                                     N_("Bopomofo (debug)"),
                                                     N_("Bopomofo input method (debug)"),
                                                     "zh_CN",
                                                     "GPL",
                                                     "Peng Huang <shawn.p.huang@gmail.com>",
                                                     PKGDATADIR"/icons/ibus-pinyin.svg",  // TODO: need a new icon
                                                     "us"));

    factory = ibus_factory_new (ibus_bus_get_connection (bus));

    if (ibus) {
        ibus_factory_add_engine (factory, "pinyin", IBUS_TYPE_PINYIN_ENGINE);
        ibus_factory_add_engine (factory, "bopomofo", IBUS_TYPE_PINYIN_ENGINE);
        ibus_bus_request_name (bus, "org.freedesktop.IBus.Pinyin", 0);
    }
    else {
        ibus_factory_add_engine (factory, "pinyin-debug", IBUS_TYPE_PINYIN_ENGINE);
        ibus_factory_add_engine (factory, "bopomofo-debug", IBUS_TYPE_PINYIN_ENGINE);
        ibus_bus_register_component (bus, component);
    }

    ibus_main ();
}

int
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("- ibus pinyin engine component");

    g_option_context_add_main_entries (context, entries, "ibus-pinyin");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    start_component ();
    return 0;
}
