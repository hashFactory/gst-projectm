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
    GST_VIDEO_CAPS_MAKE("video/x-raw, format = (string) { RGBA, BGRA }, frame-rate = (int) { 25, 30, 60 }, framerate = (int) { 25, 30, 60 }")

/* FIXME: add/remove formats you can handle */
#define AUDIO_SINK_CAPS \
    GST_AUDIO_CAPS_MAKE("audio/x-raw, " \
        "format = (string) " GST_AUDIO_NE (S16) ", " \
        "layout = (string) interleaved, " "channels = (int) { 2 }, " \
        "rate = (int) { 44100 }, " "channel-mask = (bitmask) {0x003}")



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

/*
GType
gst_projectm_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (type == 0)) {
    static const GTypeInfo info = {
      sizeof (GstProjectMClass),
      NULL,
      NULL,
      gst_projectm_class_init,
      NULL,
      NULL,
      sizeof (GstProjectM),
      0,
      (GInstanceInitFunc) gst_projectm_init,
    };

    type =
        g_type_register_static (GST_TYPE_AUDIO_VISUALIZER, "GstProjectM",
        &info, 0);
  }
  return type;
}
*/

static void gst_trido_setup_projectm(GstProjectM * projectm) {
  GstGLContext *other = gst_gl_context_get_current();
  if (other)
    return;

  GstGLDisplay *display = gst_gl_display_new_with_type(GST_GL_DISPLAY_TYPE_X11);
  GstGLContext *context = gst_gl_context_new(display);
  GError *error = NULL;

  const GstGLFuncs *gl = context->gl_vtable;

  GstGLWindow *window; //= gst_gl_window_new(display);

  gint major;
  gint minor;

    size_t width = 1920; size_t height = 1080;

 // gst_gl_context_set_window(context, window);

  gst_gl_context_create (context, 0, &error);
  //gst_gl_context_set_shared_with(context, other);
  window = gst_gl_context_get_window (context);

  gst_gl_context_get_gl_version(context, &major, &minor);

  gst_gl_window_set_preferred_size (window, width, height);
  gst_gl_window_set_render_rectangle(window, 0, 0, width, height);
  gst_gl_window_resize(window, width, height);

  gboolean out = gst_gl_context_fill_info(context, &error);
  GST_LOG_OBJECT(projectm, "context fill: %d", out);
  gst_gl_context_activate(context, true);
  GstGLFramebuffer *fbo = gst_gl_framebuffer_new (context);

  gst_gl_memory_init_once ();

  //gst_gl_framebuffer_bind(fbo);

  //gst_gl_window_draw (window);

  //GST_DEBUG_OBJECT(trido, "framebuffer width: %d height: %d\n", width, height);

  gst_gl_context_set_window(context, window);

  guint curr_con = gst_gl_context_get_current_gl_context(GST_GL_PLATFORM_GLX);

  //gst_gl_context_create()

  GST_DEBUG_OBJECT(projectm, "current context: %d\n", curr_con);


  //gst_gl_context_set_shared_with();

  char* presetPath = "/home/tristan/Documents/stuff/frontend-sdl2/build5/src/-personal";
  char* texturePath = "/home/tristan/Documents/stuff/frontend-sdl2/build5/src/-personal";

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

  //projectm_set_window_size(projectm->handle, 1920, 1080);

  //projectm_get_window_size(projectm->handle, &width, &height);

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
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

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
/*
static void
gst_projectm_clear_actors (GstProjectM * projectm)
{
  if (projectm->dis) {
    projectm_object_unref (PROJECTM_OBJECT (projectm->actor));
    projectm->actor = NULL;
  }
  if (projectm->video) {
    projectm_object_unref (PROJECTM_OBJECT (projectm->video));
    projectm->video = NULL;
  }
  if (projectm->audio) {
    projectm_object_unref (PROJECTM_OBJECT (projectm->audio));
    projectm->audio = NULL;
  }
}
*/


static void
gst_projectm_finalize (GObject * object)
{
  GstProjectM *projectm = GST_PROJECTM (object);

  // TODO
  if (projectm->framebuffer)
    free(projectm->framebuffer);
  if (projectm->handle)
    projectm_destroy(projectm->handle);
  //gst_projectm_clear_actors (projectm);

  G_OBJECT_CLASS (gst_projectm_parent_class)->finalize (object);
  //GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static gboolean
gst_projectm_setup (GstAudioVisualizer * bscope)
{
  GstProjectM *projectm = GST_PROJECTM (bscope);
  gint depth;

  // TODO
  //gst_projectm_clear_actors (projectm);

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

  //projectm->actor =
      //projectm_actor_new (GST_PROJECTM_GET_CLASS (projectm)->plugin->info->plugname);
  //projectm->video = projectm_video_new ();
  //projectm->audio = projectm_audio_new ();
  /* can't have a play without actors */

  //projectm_actor_set_video (projectm->actor, projectm->video);

  //projectm_video_set_depth (projectm->video,
  //  projectm_video_depth_enum_from_value (depth));
    bscope->req_spf = (bscope->ainfo.channels * bscope->ainfo.rate * 2) / bscope->vinfo.fps_n;
    GST_LOG_OBJECT(projectm, "%s", bscope->ainfo.finfo->description);

    projectm_set_fps(projectm->handle, bscope->vinfo.fps_n);

    projectm_set_window_size(projectm->handle, GST_VIDEO_INFO_WIDTH (&bscope->vinfo), GST_VIDEO_INFO_HEIGHT (&bscope->vinfo));

    projectm->framebuffer = (uint8_t*)malloc(GST_VIDEO_INFO_WIDTH (&bscope->vinfo) * GST_VIDEO_INFO_HEIGHT (&bscope->vinfo) * 4);

  //projectm_actor_video_negotiate (projectm->actor, 0, FALSE, FALSE);

  GST_DEBUG_OBJECT (projectm, "WxH: %dx%d, depth: %d",
      GST_VIDEO_INFO_WIDTH (&bscope->vinfo),
      GST_VIDEO_INFO_HEIGHT (&bscope->vinfo), depth);

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

  //projectm_video_set_buffer (projectm->video, GST_VIDEO_FRAME_PLANE_DATA (video, 0));
  //projectm_video_set_pitch (projectm->video, GST_VIDEO_FRAME_PLANE_STRIDE (video,0));

  channels = GST_AUDIO_INFO_CHANNELS (&bscope->ainfo);
  num_samples =
      amap.size / (GST_AUDIO_INFO_CHANNELS (&bscope->ainfo) * sizeof (gint16));
   GstMemory *mem =  gst_buffer_get_all_memory(audio);
  GST_DEBUG_OBJECT(projectm, "mem size: %lu", mem->size);

  gst_buffer_map (audio, &amap, GST_MAP_READ);
  //adata = (gint16 *) malloc(amap.size*sizeof(gint16)*2);
  //memcpy(adata, amap.data, amap.size);

  gst_video_frame_map(video, &video->info, video->buffer, GST_MAP_READWRITE);
/*
  if (channels == 2) {
    for (i = 0; i < PROJECTM_SAMPLES; i++) {
      ldata[i] = *adata++;
      rdata[i] = *adata++;
    }
  } else {
    for (i = 0; i < PROJECTM_SAMPLES; i++) {
      ldata[i] = *adata;
      rdata[i] = *adata++;
    }
  }*/

  GST_DEBUG_OBJECT(projectm, "samples: %lu, offset: %lu, offset end: %lu, vrate: %d, fps: %d, req_spf: %d", amap.size / 8, audio->offset , audio->offset_end, bscope->ainfo.rate, bscope->vinfo.fps_n, bscope->req_spf);
  projectm_pcm_add_int16(projectm->handle, (gint16 *) amap.data, amap.size/4, PROJECTM_STEREO);
  //projectm_pcm_get_max_samples()

  GST_DEBUG_OBJECT(projectm, "audio data: %d %d %d %d", ((gint16*)amap.data)[100], ((gint16*)amap.data)[101], ((gint16*)amap.data)[102], ((gint16*)amap.data)[103]);
  //vrate = bscope->vinfo.rate;

  //bscope->vinfo
  //gst_video_frame_map()

  const GstGLFuncs *gl = projectm->context->gl_vtable;

  GST_DEBUG_OBJECT (projectm, "transform_frame");

  GRand *grand = g_rand_new_with_seed(49);
  size_t width, height;
  projectm_get_window_size(projectm->handle, &width, &height);

  //size_t texture_size = projectm_get_texture_size(pmhandle);

//  int length = 4000;

  //projectm_write_debug_image_on_next_frame(pmhandle);
  //gst_gl_context_activate(trido->context, true);
    gl->Viewport(0, 0, width, height);
      //gst_gl_context_clear_framebuffer(trido->context);


  //gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  projectm_opengl_render_frame(projectm->handle);
  _check_gl_error(projectm->context, projectm);


  //uint8_t *data = malloc(sizeof(uint8_t) * width * height * 4);
  uint8_t *data = projectm->framebuffer;

  //gl->BindTexture()
  //gl->ReadBuffer(  GL_FRONT_AND_BACK);
  gl->ReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
  GST_DEBUG_OBJECT(projectm, "%d %d %d %d", data[0], data[1], data[2], data[3]);

  _check_gl_error(projectm->context, projectm);

  //projectm_write_debug_image_on_next_frame(pmhandle, "out.png");
  //gst_gl_context_swap_buffers(trido->context);
  //gst_gl_window_controls_viewport(trido->window);
  //gst_gl_window_draw(trido->window);

  //gst_gl_context_activate(trido->context, false);
  //gst_gl_window_show(trido->window);

  gst_gl_context_swap_buffers(projectm->context);

  //guint curr_con = gst_gl_context_get_current_gl_context(GST_GL_PLATFORM_ANY);
  //char dbg[50];
  //g_snprintf(dbg, 50, "%d", curr_con);
  //GST_DEBUG_OBJECT (trido, "%s", dbg);

  //uint8_t **rdata = (uint8_t**)malloc(sizeof(uint8_t*) * 4);

  //gst_video_info_map

  //gst_video_frame_map(outframe, inframe);

  //gst_video_info
  //memcpy(((uint8_t*)(outframe->data[0])), data, width*height*4);
  //outframe->data[0] = (gpointer)data;

  uint8_t * vdata = ((uint8_t*)(video->data[0]));

  memcpy(((uint8_t*)(video->data[0])), data, width*height*4);

  /*// RGBA
  for (int r = 0; r < width*height*4; r+=4) {
    vdata[r+3] = data[r];
    //vdata[r+1] = data[r+1];
    //vdata[r+2] = data[r+2];
    vdata[r] = data[r+3];
  }
  */
  for (int r = 0; r < width*height*4; r+=4) {
    vdata[r+3] = data[r];
    vdata[r+2] = data[r+2];
    vdata[r+1] = data[r+1];
    vdata[r] = data[r+3];
  }

  //GST_DEBUG_OBJECT(trido, "%s %lu", outframe->info.finfo->name, outframe->info.offset[1]);

    GST_DEBUG_OBJECT(projectm, "v2 %d %d\n", GST_VIDEO_FRAME_N_PLANES(video), ((uint8_t*)(GST_VIDEO_FRAME_PLANE_DATA(video, 0)))[0]);

  /*projectm_audio_samplepool_input_channel (projectm->audio->samplepool,
      lbuf,
      vrate, PROJECTM_AUDIO_SAMPLE_FORMAT_S16,
      (char *) PROJECTM_AUDIO_CHANNEL_LEFT);
  projectm_audio_samplepool_input_channel (projectm->audio->samplepool, rbuf,
      vrate, PROJECTM_AUDIO_SAMPLE_FORMAT_S16,
      (char *) PROJECTM_AUDIO_CHANNEL_RIGHT);
*/
  //projectm_object_unref (PROJECTM_OBJECT (lbuf));
  //projectm_object_unref (PROJECTM_OBJECT (rbuf));

  //projectm_audio_analyze (projectm->audio);
  //projectm_actor_run (projectm->actor, projectm->audio);
  //projectm_video_set_buffer (projectm->video, NULL);

  GST_DEBUG_OBJECT (projectm, "rendered one frame");
done:
  gst_buffer_unmap (audio, &amap);
  //gst_buffer_unref (audio);
  //gst_buffer_remove_all_memory(audio);

  return res;
}


