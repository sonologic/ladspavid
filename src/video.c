#include <gst/gst.h>
#include <glib.h>


static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}



int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *pipeline2, *source, *decoder, *color, *freezer, *ladspa, *sink, *sink2;
  GstBus *bus;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Check input arguments */
  if (argc != 2) {
    g_printerr ("Usage: %s <image filename>\n", argv[0]);
    return -1;
  }


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("ladspavid");
  pipeline2 = gst_pipeline_new ("ladspavid2");
  //source   = gst_element_factory_make ("filesrc",       "file-source");
  source   = gst_element_factory_make ("videotestsrc",       "video-test-source");
  if(!source) { printf("unable to create source\n"); exit(-1); }
  decoder  = gst_element_factory_make ("decodebin",     "image-decoder");
  if(!decoder) { printf("unable to create decoder\n"); exit(-1); }
  color    = gst_element_factory_make ("ffmpegcolorspace",     "color-spacer");
  if(!color) { printf("unable to create color\n"); exit(-1); }
  freezer  = gst_element_factory_make ("imagefreeze",     "image-freezer");
  if(!freezer) { printf("unable to create freezer\n"); exit(-1); }
  ladspa   = gst_element_factory_make ("ladspavidflanger", "ladspa-vid");
  if(!ladspa) { printf("unable to create ladspa\n"); exit(-1); }
  sink     = gst_element_factory_make ("autovideosink", "video-output");
  if(!sink) { printf("unable to create sink\n"); exit(-1); }
  sink2    = gst_element_factory_make ("autovideosink", "video-output-2");
  if(!sink2) { printf("unable to create sink2\n"); exit(-1); }

  if (!pipeline || !source || !decoder || !color || !freezer || !ladspa || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  //g_object_set (G_OBJECT (source), "location", argv[1], NULL);
  //g_object_set (G_OBJECT (ladspa), "silent", 0, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline2));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
  //gst_bin_add_many (GST_BIN (pipeline),
  //                  source, decoder, color, freezer, ladspa, sink, NULL);

  gst_bin_add_many (GST_BIN (pipeline),
                    source, ladspa, sink, NULL);
  gst_bin_add_many (GST_BIN (pipeline2),
                    source, sink2, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  //gst_element_link (source, decoder);
  //gst_element_link_many (color, freezer, ladspa, sink, NULL);
  //g_signal_connect (decoder, "pad-added", G_CALLBACK (on_pad_added), color);
  gst_element_link_many (source, ladspa, sink, NULL);
  gst_element_link_many (source, sink2, NULL);
  //gst_element_link_many (source, sink2, NULL);

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  gst_element_set_state (pipeline2, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

