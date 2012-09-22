/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2012 gmc <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-ladspavidflanger
 *
 * FIXME:Describe ladspavidflanger here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! ladspavidflanger ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gst/gst.h>
#include "gstladspavidflanger.h"

GST_DEBUG_CATEGORY_STATIC (gst_ladspa_vid_flanger_debug);
#define GST_CAT_DEFAULT gst_ladspa_vid_flanger_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_RATE,
  PROP_DELAY,
  PROP_MAX_SLOW,
  PROP_LFO,
  PROP_FEEDBACK
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

GST_BOILERPLATE (GstLadspaVidFlanger, gst_ladspa_vid_flanger, GstElement,
    GST_TYPE_ELEMENT);

static void gst_ladspa_vid_flanger_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_ladspa_vid_flanger_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_ladspa_vid_flanger_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_ladspa_vid_flanger_chain (GstPad * pad, GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_ladspa_vid_flanger_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "LadspaVidFlanger",
    "Effect/Video",
    "Feed video through ladspa plugins to create video-glitching effects inspired by http://www.hellocatfood.com/2009/11/16/databending-using-audacity/",
    "Koen Martens <gmc@sonologic.nl>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the ladspavidflanger's class */
static void
gst_ladspa_vid_flanger_class_init (GstLadspaVidFlangerClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_ladspa_vid_flanger_set_property;
  gobject_class->get_property = gst_ladspa_vid_flanger_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_RATE,
      g_param_spec_long ("rate", "Rate", "LADSPA sample frequency",
    		  1,128000,44100,
    		  G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
  g_object_class_install_property (gobject_class, PROP_DELAY,
      g_param_spec_float ("delay", "Delay", "Delay base (ms)",
    		  0.1, 25.0, 0.5,
    		  G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
  g_object_class_install_property (gobject_class, PROP_MAX_SLOW,
      g_param_spec_float ("max_slow", "Max slowdown (ms)", "Max slowdown (ms)",
    		  0.0, 10.0, 0.1,
    		  G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
  g_object_class_install_property (gobject_class, PROP_LFO,
      g_param_spec_float ("lfo", "LFO", "LFO frequency (Hz)",
    		  0.05, 100.0, 0.1,
    		  G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
  g_object_class_install_property (gobject_class, PROP_FEEDBACK,
      g_param_spec_float ("feedback", "Feedback", "Feedback",
    		  -1.0, 1.0, -0.3,
    		  G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_ladspa_vid_flanger_init (GstLadspaVidFlanger * filter,
    GstLadspaVidFlangerClass * gclass)
{
  memset(&filter->wrapper,0,sizeof(ladspa_wrapper));
  printf("init plugin\n");

  ladspa_init_plugin("/usr/lib/ladspa/flanger_1191.so",&filter->wrapper);

  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");


  gst_pad_set_setcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_ladspa_vid_flanger_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_ladspa_vid_flanger_chain));

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (filter->srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->silent = FALSE;
}

static void
gst_ladspa_vid_flanger_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstLadspaVidFlanger *filter = GST_LADSPAVIDFLANGER (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_DELAY:
      filter->wrapper.delay = g_value_get_float(value);
      break;
    case PROP_MAX_SLOW:
      filter->wrapper.max_slow = g_value_get_float(value);
      break;
    case PROP_LFO:
      filter->wrapper.lfo = g_value_get_float(value);
      break;
    case PROP_FEEDBACK:
      filter->wrapper.feedback = g_value_get_float(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_ladspa_vid_flanger_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstLadspaVidFlanger *filter = GST_LADSPAVIDFLANGER (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_DELAY:
      g_value_set_float (value, filter->wrapper.delay);
      break;
    case PROP_MAX_SLOW:
      g_value_set_float (value, filter->wrapper.max_slow);
      break;
    case PROP_LFO:
      g_value_set_float (value, filter->wrapper.lfo);
      break;
    case PROP_FEEDBACK:
      g_value_set_float (value, filter->wrapper.feedback);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_ladspa_vid_flanger_set_caps (GstPad * pad, GstCaps * caps)
{
  GstLadspaVidFlanger *filter;
  GstPad *otherpad;

  filter = GST_LADSPAVIDFLANGER (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  return gst_pad_set_caps (otherpad, caps);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_ladspa_vid_flanger_chain (GstPad * pad, GstBuffer * buf)
{
  GstLadspaVidFlanger *filter;

  filter = GST_LADSPAVIDFLANGER (GST_OBJECT_PARENT (pad));

  //if (filter->silent == FALSE)
  //  g_print ("I'm plugged, therefore I'm in.\n");

  ladspa_run_plugin(&filter->wrapper, buf->size, buf->data);
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
ladspavidflanger_init (GstPlugin * ladspavidflanger)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template ladspavidflanger' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_ladspa_vid_flanger_debug, "ladspavidflanger",
      0, "Template ladspavidflanger");

  gst_controller_init(NULL, NULL);

  return gst_element_register (ladspavidflanger, "ladspavidflanger", GST_RANK_NONE,
      GST_TYPE_LADSPAVIDFLANGER);
}

#ifndef PACKAGE
#define PACKAGE "ladspavid"
#endif

/* gstreamer looks for this structure to register ladspavids
 *
 * exchange the string 'Template ladspavid' with your ladspavid description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "ladspavid flanger",
    "Ladspa video flanger",
    ladspavidflanger_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
