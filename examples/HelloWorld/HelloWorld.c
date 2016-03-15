#include <gst/gst.h>

/*
 *  Global objects are usually a bad thing. For the purpose of this
 *  example, we will use them, however.
 *
 */

GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;

static void seek_to_time(GstElement *pipeline, gint64 time_nanoseconds) {
    if (!gst_element_seek(pipeline,
                1.0,
                GST_FORMAT_TIME,
                GST_SEEK_FLAG_FLUSH,
                GST_SEEK_TYPE_SET,
                time_nanoseconds,
                GST_SEEK_TYPE_NONE,
                GST_CLOCK_TIME_NONE))
        g_print ("Seek failed!\n");
}

static gboolean cb_print_position(GstElement *pipeline) {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos, len;

    if (gst_element_query_position (pipeline, &fmt, &pos)
            && gst_element_query_duration (pipeline, &fmt, &len)) {
        g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\n",
                GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
    }

    if (pos/1000000000%10 == 0) {
        g_print("position=%ld, now seek to 10s after.\n", pos/1000000000);
        seek_to_time(pipeline, pos + 10000000000);
    }

    /* call me again */
    return TRUE;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = data;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            g_main_loop_quit(loop);
            break;

        case GST_MESSAGE_ERROR:
            {
            gchar *debug;
            GError *err;

            gst_message_parse_error(msg, &err, &debug);
            g_free(debug);

            g_print("Error: %s\n", err->message);
            g_error_free(err);

            g_main_loop_quit(loop);
            break;
            }

        default:
                                break;
    }
    return TRUE;
}

static void new_pad(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;

    /* We can now link this pad with the audio decoder */
    g_print("Dynamic pad created, linking parser/decoder\n");

    sinkpad = gst_element_get_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);

    gst_object_unref(sinkpad);
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstBus *bus;

    /* initialize GStreamer */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* check input arguments */
    if (argc != 2) {
        g_print("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
        return -1;
    }

    /* create elements */
    pipeline = gst_pipeline_new("audio-player");
    source = gst_element_factory_make("filesrc", "file-source");
    parser = gst_element_factory_make("oggdemux", "ogg-parser");
    decoder = gst_element_factory_make("vorbisdec", "vorbis-decoder");
    conv = gst_element_factory_make("audioconvert", "converter");
    sink = gst_element_factory_make("alsasink", "alsa-output");
    if (!pipeline || !source || !parser || !decoder || !conv || !sink) {
        g_print("One element could not be created\n");
        return -1;
    }

    /* 
     * set filename property on the file source. Also add a message
     * handler.
     *
     */
    g_object_set(G_OBJECT(source), "location", argv[1], NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    /* put all elements in a bin */
    gst_bin_add_many(GST_BIN(pipeline),
            source, parser, decoder, conv, sink, NULL);

    /* 
     * link together - note that we cannot link the parser and
     * decoder yet, becuse the parser uses dynamic pads. For that,
     * we set a pad-added signal handler.
     *
     */
    gst_element_link(source, parser);
    gst_element_link_many(decoder, conv, sink, NULL);
    g_signal_connect(parser, "pad-added", G_CALLBACK(new_pad), NULL);

    /* Now set to playing and iterate. */
    g_print("Setting to PLAYING\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Query position every 200ms */
    g_timeout_add (1000, (GSourceFunc) cb_print_position, pipeline);

    g_print("Running\n");
    g_main_loop_run(loop);

    /* clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(pipeline));
    return 0;
}
