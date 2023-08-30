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

#ifndef __GST_PROJECTM_H__
#define __GST_PROJECTM_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <gst/video/video.h>
#include <gst/video/gstvideopool.h>
#include <gst/audio/audio.h>

#include <projectM-4/projectM.h>

#include <gst/gl/gl.h>
#include <gst/gl/gstglfuncs.h>

#include <gst/pbutils/gstaudiovisualizer.h>

G_BEGIN_DECLS

#define GST_TYPE_PROJECTM (gst_projectm_get_type())
#define GST_IS_PROJECTM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PROJECTM))
#define GST_PROJECTM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PROJECTM,GstProjectM))
#define GST_IS_PROJECTM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PROJECTM))
#define GST_PROJECTM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PROJECTM,GstProjectMClass))
#define GST_PROJECTM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_PROJECTM, GstProjectMClass))

typedef struct _GstProjectM GstProjectM;
typedef struct _GstProjectMClass GstProjectMClass;

struct _GstProjectM
{
  GstAudioVisualizer element;

  /* libvisual stuff */
  GstGLDisplay *display;
  GstGLWindow *window;
  GstGLContext *context;
  gchar* preset;
  gint texture;

  projectm_handle handle;
  uint8_t *framebuffer;
};

struct _GstProjectMClass
{
  GstAudioVisualizerClass parent_class;
};

static void gst_projectm_class_init (GstProjectMClass *g_class);

GType gst_projectm_get_type (void);

G_END_DECLS

#endif /* __GST_PROJECTM_H__ */

