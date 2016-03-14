#include <gst/gst.h>

/*
 * Global objects are usually a bad thing. For the purpose of this
 * example, we will use them, however.
 *
 */

GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GmainLoop *loop = data;

    switch(GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
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

static void
new_pad(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;

    /* We can now link this pad with the audio decoder */
    g_print("Dynamic pad created, linking parser/decoder\n");

    sinkpad = gst_element_get_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);

    gst_object_unref(sinkpad);
}

int
main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstBus *bus;

    /* initialize GStreamer */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* check input arguments */
    if (argc !=2 ) {
        g_print("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);

        return -1;
    }

    /* create elements */
    pipleline = gst_pipeline_new("audio-player");
    source = gst_element_factory_make("filesrc", "file-source");
    parser = gst_element_factory_make("oggdemux", "ogg-parser");
    decoder = gst_element_factory_make("vorbisdec", "vorbis-decoder");
    conv = gst_element_factory_make("audioconvert", "converter");
    sink = gst_element_factory_make("alsasink", "alsa-output");
}
