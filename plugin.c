/* GStreamer
 * Copyright (C) 2004 Benjamin Otte <otte@gnome.org>
 *               2012 Stefan Sauer <ensonic@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "projectm.h"

GST_DEBUG_CATEGORY (projectm_debug);
#define GST_CAT_DEFAULT (projectm_debug)

static void
projectm_log_handler (const char *message, const char *funcname, void *priv)
{
  GST_CAT_LEVEL_LOG (projectm_debug, (GstDebugLevel) GPOINTER_TO_INT (priv),
      NULL, "%s - %s", funcname, message);
}

/*
 * Replace invalid chars with _ in the type name
 */
static void
make_valid_name (gchar * name)
{
  static const gchar extra_chars[] = "-_+";
  gchar *p = name;

  for (; *p; p++) {
    gint valid = ((p[0] >= 'A' && p[0] <= 'Z') ||
        (p[0] >= 'a' && p[0] <= 'z') ||
        (p[0] >= '0' && p[0] <= '9') || strchr (extra_chars, p[0]));
    if (!valid)
      *p = '_';
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  guint i, count;
  GType type;
  gchar *name = g_strdup_printf ("projectm");
  make_valid_name (name);

    GTypeInfo info = {
      sizeof (GstProjectMClass),
      NULL,
      NULL,
      gst_projectm_class_init,
      NULL,
      NULL,
      sizeof (GstProjectM),
      0,
      NULL
    };

    GST_DEBUG_CATEGORY_INIT (projectm_debug, "projectm", 0,
      "projectm audio visualisations");

   // type = g_type_register_static (GST_TYPE_PROJECTM, name, &info, 0);
    gst_element_register (plugin, name, GST_RANK_NONE, GST_TYPE_PROJECTM);

      /*name = g_strdup_printf ("GstProjectM%s", ref->info->plugname);
      make_valid_name (name);
      type = g_type_register_static (GST_TYPE_PROJECTM, name, &info, 0);
      g_free (name);
*/
      /*name = g_strdup_printf ("projectm_%s", "me");
      make_valid_name (name);
      if (!gst_element_register (plugin, name, GST_RANK_NONE, type)) {
        g_free (name);
        return FALSE;
      }
      g_free (name);
*/
  /*
  VisList *list;

  GST_DEBUG_CATEGORY_INIT (projectm_debug, "projectm", 0,
      "projectm audio visualisations");

#ifdef PROJECTM_PLUGINSBASEDIR
  gst_plugin_add_dependency_simple (plugin, "HOME/.projectm/actor",
      PROJECTM_PLUGINSBASEDIR "/actor", NULL, GST_PLUGIN_DEPENDENCY_FLAG_NONE);
#endif

  projectm_log_set_verboseness (PROJECTM_LOG_VERBOSENESS_LOW);
  projectm_log_set_info_handler (projectm_log_handler,
      GINT_TO_POINTER (GST_LEVEL_INFO));
  projectm_log_set_warning_handler (projectm_log_handler,
      GINT_TO_POINTER (GST_LEVEL_WARNING));
  projectm_log_set_critical_handler (projectm_log_handler,
      GINT_TO_POINTER (GST_LEVEL_ERROR));
  projectm_log_set_error_handler (projectm_log_handler,
      GINT_TO_POINTER (GST_LEVEL_ERROR));

  if (!projectm_is_initialized ())
    if (projectm_init (NULL, NULL) != 0)
      return FALSE;

  list = projectm_actor_get_list ();

  count = projectm_collection_size (PROJECTM_COLLECTION (list));

  for (i = 0; i < count; i++) {
    VisPluginRef *ref = projectm_list_get (list, i);
    VisPluginData *visplugin = NULL;
    gboolean skip = FALSE;
    GType type;
    gchar *name;
    GTypeInfo info = {
      sizeof (GstProjectMClass),
      NULL,
      NULL,
      gst_projectm_class_init,
      NULL,
      ref,
      sizeof (GstProjectM),
      0,
      NULL
    };

    visplugin = projectm_plugin_load (ref);

    if (ref->info->plugname == NULL)
      continue;

    // Blacklist some plugins
    if (strcmp (ref->info->plugname, "gstreamer") == 0 ||
        strcmp (ref->info->plugname, "gdkpixbuf") == 0) {
      skip = TRUE;
    } else {
      // Ignore plugins that only support GL output for now
      skip = gst_projectm_actor_plugin_is_gl (visplugin->info->plugin,
          visplugin->info->plugname);
    }

    projectm_plugin_unload (visplugin);

    if (!skip) {
      name = g_strdup_printf ("GstProjectM%s", ref->info->plugname);
      make_valid_name (name);
      type = g_type_register_static (GST_TYPE_PROJECTM, name, &info, 0);
      g_free (name);

      name = g_strdup_printf ("projectm_%s", ref->info->plugname);
      make_valid_name (name);
      if (!gst_element_register (plugin, name, GST_RANK_NONE, type)) {
        g_free (name);
        return FALSE;
      }
      g_free (name);
    }
  }
*/
  return TRUE;
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "projectm_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "GstProjectM_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    projectm,
    "projectm visualization plugins",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
