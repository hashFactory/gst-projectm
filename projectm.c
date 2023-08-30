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
#include <projectM-4/projectM.h>

#include <gst/audio/audio-format.h>
#include <gst/pbutils/gstaudiovisualizer.h>

#include <gst/gl/gl.h>
#include <gst/gl/gstglfuncs.h>

GST_DEBUG_CATEGORY_STATIC (gst_projectm_debug);
#define GST_CAT_DEFAULT gst_projectm_debug

//GST_DEBUG_CATEGORY_EXTERN (projectm_debug);
//#define GST_CAT_DEFAULT (projectm_debug)

//static void gst_projectm_set_property (GObject * object,
//    guint property_id, const GValue * value, GParamSpec * pspec);
//static void gst_projectm_get_property (GObject * object,
//    guint property_id, GValue * value, GParamSpec * pspec);

/* amounf of samples before we can feed libprojectm */
#define PROJECTM_SAMPLES  512

#if G_BYTE_ORDER == G_BIG_ENDIAN
#define RGB_ORDER_CAPS "xRGB, RGB"
#else
#define RGB_ORDER_CAPS "BGRx, BGR"
#endif

/* FIXME: add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("video/x-raw, format = (string) { RGBA, BGRA }, framerate=(fraction)[0/1,MAX]"  )

/* FIXME: add/remove formats you can handle */
#define AUDIO_SINK_CAPS \
    GST_AUDIO_CAPS_MAKE("audio/x-raw, " \
        "format = (string) " GST_AUDIO_NE (S16) ", " \
        "layout = (string) interleaved, " "channels = (int) { 2 }, " \
        "rate = (int) { 44100 }, " "channel-mask = (bitmask) { 0x0003 }")



static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    //GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (" { " RGB_ORDER_CAPS ", RGB16, RGBA } "))
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (" { RGBA } "))

);

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw, "
        "format = (string) " GST_AUDIO_NE (S16) ", "
        "layout = (string) interleaved, " "channels = (int) { 2 }, "
        "rate = (int) { 44100 }, " "channel-mask = (bitmask) {0x003}")
    );

enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_LOCATION
};

G_DEFINE_TYPE_WITH_CODE (GstProjectM, gst_projectm, GST_TYPE_AUDIO_VISUALIZER,
    GST_DEBUG_CATEGORY_INIT (gst_projectm_debug, "projectm", 0,
        "debug category for projectm element"));

static void gst_projectm_init (GstProjectM * projectm);
static void gst_projectm_finalize (GObject * object);

static gboolean gst_projectm_setup (GstAudioVisualizer * bscope);
static gboolean gst_projectm_render (GstAudioVisualizer * bscope,
    GstBuffer * audio, GstVideoFrame * video);

static GstElementClass *parent_class = NULL;

static void
_check_gl_error (GstGLContext * context, gpointer data)
{
  GLuint error = context->gl_vtable->GetError ();
  if (error != GL_NONE)
    g_print("GL error 0x%x encountered during processing\n",
      error);
}

static void gst_trido_setup_projectm(GstProjectM * projectm) {
  GstGLContext *other = gst_gl_context_get_current();
  if (other)
    return;

  GstGLDisplay *display = gst_gl_display_new_with_type(GST_GL_DISPLAY_TYPE_ANY);
  GstGLContext *context = gst_gl_context_new(display);
  GError *error = NULL;

  GstGLWindow *window;

  gint major;
  gint minor;

  GstAudioVisualizer *scope = GST_AUDIO_VISUALIZER(projectm);

  //  scope->vinfo.fps_n = 60;
    size_t width = scope->vinfo.width; size_t height = scope->vinfo.height;
    GST_LOG_OBJECT(projectm, "fps: %d", scope->vinfo.fps_n);

 // gst_gl_context_set_window(context, window);

  gst_gl_context_create (context, 0, &error);
  window = gst_gl_context_get_window (context);

  gst_gl_context_get_gl_version(context, &major, &minor);

  gst_gl_window_set_preferred_size (window, width, height);
  gst_gl_window_set_render_rectangle(window, 0, 0, width, height);
  gst_gl_window_resize(window, width, height);

  //gboolean out = gst_gl_context_fill_info(context, &error);
  //GST_LOG_OBJECT(projectm, "context fill: %d", out);
  gst_gl_context_activate(context, true);
  GstGLFramebuffer *fbo = gst_gl_framebuffer_new (context);

  gst_gl_memory_init_once ();

  //gst_gl_framebuffer_bind(fbo);

  //gst_gl_window_draw (window);

  //GST_DEBUG_OBJECT(trido, "framebuffer width: %d height: %d\n", width, height);

  gst_gl_context_set_window(context, window);

  guint curr_con = gst_gl_context_get_current_gl_context(GST_GL_PLATFORM_GLX);

  GST_DEBUG_OBJECT(projectm, "current context: %d\n", curr_con);

  //projectm_settings settings;

  projectm->display = display;
  projectm->window = window;
  projectm->context = context;

  projectm_handle projectMHandle = projectm_create();
  if (!projectMHandle)
  {
    GST_LOG("Could not create instance");
    //gst_debug_log();
  }
  else {
    GST_LOG("Created instance!");
  }

  projectm->handle = projectMHandle;

  //GST_DEBUG_OBJECT(projectm, "width: %zu height: %zu\n", width, height);

}


void
gst_projectm_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstProjectM *projectm = GST_PROJECTM (object);

  GST_DEBUG_OBJECT (projectm, "set_property");

  switch (property_id) {
    case PROP_LOCATION:
      projectm->preset = g_strdup(g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_projectm_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstProjectM *projectm = GST_PROJECTM (object);

  GST_DEBUG_OBJECT (projectm, "get_property");

  switch (property_id) {
    case PROP_LOCATION:
      g_value_set_string(value, g_strdup("/home/tristan/Documents/gen/cool_dots_homemade.milk"));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_projectm_class_init (GstProjectMClass *g_class)
{
  GObjectClass *gobject_class = (GObjectClass *) g_class;
  GstElementClass *element_class = (GstElementClass *) g_class;
  GstAudioVisualizerClass *scope_class = (GstAudioVisualizerClass *) g_class;
  GstProjectMClass *klass = (GstProjectMClass *) g_class;

  //klass = class_data;

  //if (class_data == NULL) {
  //  parent_class = g_type_class_peek_parent (g_class);
  //} else {
    //gchar *longname = g_strdup_printf ("libprojectm plugin v.%s", projectm_get_version_string(klass->handle));

    /* FIXME: improve to only register what plugin supports? */
    //gst_element_class_add_static_pad_template (element_class, &src_template);
    //gst_element_class_add_static_pad_template (element_class, &sink_template);
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gst_caps_from_string (AUDIO_SINK_CAPS)));


    /*gst_element_class_set_metadata (element_class,
        longname, "Visualization",
        klass->plugin->info->about, "Tristan Charpentier <tristan_charpentier@hotmail.com>");
*/
    //g_free (longname);
  //}

   gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "ProjectM Visualizer", "Generic", "A plugin for visualizing music using ProjectM",
      "Tristan Charpentier <tristan_charpentier@hotmail.com>");

  gobject_class->set_property = gst_projectm_set_property;
  gobject_class->get_property = gst_projectm_get_property;

  g_object_class_install_property (gobject_class, PROP_LOCATION,
      g_param_spec_string ("preset", "Preset file Location",
          "Location of the MilkDrop preset", "/home/tristan/dev/gst/trido/presets/Supernova/Shimmer/EoS - starburst 05 phasing.milk",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gobject_class->finalize = gst_projectm_finalize;

  scope_class->setup = GST_DEBUG_FUNCPTR (gst_projectm_setup);
  scope_class->render = GST_DEBUG_FUNCPTR (gst_projectm_render);
}

static void
gst_projectm_init (GstProjectM * projectm)
{
  /* do nothing */
}

static void
gst_projectm_finalize (GObject * object)
{
  GstProjectM *projectm = GST_PROJECTM (object);

  if (projectm->framebuffer)
    free(projectm->framebuffer);
  if (projectm->handle)
    projectm_destroy(projectm->handle);

  G_OBJECT_CLASS (gst_projectm_parent_class)->finalize (object);
}

static gboolean
gst_projectm_setup (GstAudioVisualizer * bscope)
{
  GstProjectM *projectm = GST_PROJECTM (bscope);
  gint depth;

  gst_trido_setup_projectm(projectm);

  if (projectm->preset)
    projectm_load_preset_file(projectm->handle, projectm->preset, false);
  else
    projectm_load_preset_file(projectm->handle, "/home/tristan/Documents/gen/cool_dots_homemade.milk", false);

  /* FIXME: we need to know how many bits we actually have in memory */
  depth = bscope->vinfo.finfo->pixel_stride[0];
  if (bscope->vinfo.finfo->bits >= 8) {
    depth *= 8;
  }

  //projectm_actor_set_video (projectm->actor, projectm->video);

    bscope->req_spf = (bscope->ainfo.channels * bscope->ainfo.rate * 2) / bscope->vinfo.fps_n;
    GST_LOG_OBJECT(projectm, "%s", bscope->ainfo.finfo->description);

    projectm_set_fps(projectm->handle, bscope->vinfo.fps_n);
    projectm_set_window_size(projectm->handle, GST_VIDEO_INFO_WIDTH (&bscope->vinfo), GST_VIDEO_INFO_HEIGHT (&bscope->vinfo));

    projectm->framebuffer = (uint8_t*)malloc(GST_VIDEO_INFO_WIDTH (&bscope->vinfo) * GST_VIDEO_INFO_HEIGHT (&bscope->vinfo) * 4);

  GST_DEBUG_OBJECT (projectm, "WxH: %dx%d, depth: %d, fps: %d/%d",
      GST_VIDEO_INFO_WIDTH (&bscope->vinfo),
      GST_VIDEO_INFO_HEIGHT (&bscope->vinfo), depth,
       (bscope->vinfo.fps_n), (bscope->vinfo.fps_d));

  return TRUE;
  /* ERRORS */

}

static gboolean
gst_projectm_render (GstAudioVisualizer * bscope, GstBuffer * audio,
    GstVideoFrame * video)
{
  GstProjectM *projectm = GST_PROJECTM (bscope);
  GstMapInfo amap;
  gint16 *adata;
  gint i, channels;
  gboolean res = TRUE;
  //VisBuffer *lbuf, *rbuf;
  guint32 vrate;
  guint num_samples;

  // AUDIO
  channels = GST_AUDIO_INFO_CHANNELS (&bscope->ainfo);
  num_samples = amap.size / (GST_AUDIO_INFO_CHANNELS (&bscope->ainfo) * sizeof (gint16));

  GstMemory *mem =  gst_buffer_get_all_memory(audio);
  GST_DEBUG_OBJECT(projectm, "mem size: %lu", mem->size);

  gst_buffer_map (audio, &amap, GST_MAP_READ);

  GST_DEBUG_OBJECT(projectm, "samples: %lu, offset: %lu, offset end: %lu, vrate: %d, fps: %d, req_spf: %d", amap.size / 8, audio->offset , audio->offset_end, bscope->ainfo.rate, bscope->vinfo.fps_n, bscope->req_spf);

  projectm_pcm_add_int16(projectm->handle, (gint16 *) amap.data, amap.size/4, PROJECTM_STEREO);

  GST_DEBUG_OBJECT(projectm, "audio data: %d %d %d %d", ((gint16*)amap.data)[100], ((gint16*)amap.data)[101], ((gint16*)amap.data)[102], ((gint16*)amap.data)[103]);

  gst_video_frame_map(video, &video->info, video->buffer, GST_MAP_READWRITE);

  const GstGLFuncs *gl = projectm->context->gl_vtable;

  size_t width, height;
  projectm_get_window_size(projectm->handle, &width, &height);

  //projectm_write_debug_image_on_next_frame(pmhandle);
  //gst_gl_context_activate(trido->context, true);
    gl->Viewport(0, 0, width, height);

  //gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  projectm_opengl_render_frame(projectm->handle);
  _check_gl_error(projectm->context, projectm);

  uint8_t *data = projectm->framebuffer;

  gl->ReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
  GST_DEBUG_OBJECT(projectm, "%d %d %d %d", data[0], data[1], data[2], data[3]);

  _check_gl_error(projectm->context, projectm);

  gst_gl_context_swap_buffers(projectm->context);

  //gst_video_info
  //memcpy(((uint8_t*)(outframe->data[0])), data, width*height*4);
  //outframe->data[0] = (gpointer)data;

  uint8_t * vdata = ((uint8_t*)(video->data[0]));

  //memcpy(((uint8_t*)(video->data[0])), data, width*height*4);

  /*// RGBA
  for (int r = 0; r < width*height*4; r+=4) {
    vdata[r+3] = data[r];
    //vdata[r+1] = data[r+1];
    //vdata[r+2] = data[r+2];
    vdata[r] = data[r+3];
  }
  */
  // BGRA
  for (int r = 0; r < width*height*4; r+=4) {
    vdata[r+3] = data[r];
    vdata[r+2] = data[r+2];
    vdata[r+1] = data[r+1];
    vdata[r] = data[r+3];
  }

  //GST_DEBUG_OBJECT(trido, "%s %lu", outframe->info.finfo->name, outframe->info.offset[1]);

    GST_DEBUG_OBJECT(projectm, "v2 %d %d\n", GST_VIDEO_FRAME_N_PLANES(video), ((uint8_t*)(GST_VIDEO_FRAME_PLANE_DATA(video, 0)))[0]);

  //projectm_object_unref (PROJECTM_OBJECT (lbuf));
  //projectm_object_unref (PROJECTM_OBJECT (rbuf));

  GST_DEBUG_OBJECT (projectm, "rendered one frame");
done:
  gst_buffer_unmap (audio, &amap);

  return res;
}


