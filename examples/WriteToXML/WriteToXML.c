#include <stdlib.h>
#include <gst/gst.h>

gboolean playing;
int main(int argc, char *argv[])
{
    GstElement *filesrc, *osssink, *queue, *queue2, *decode;
    GstElement *bin;
    //Gst;Element *thread, *thread2;
    gst_init (&argc,&argv);

    if (argc != 2) {
        g_print ("usage: %s <mp3 filename>\n", argv[0]);
        exit (-1);
    }

    /* create a new thread to hold the elements */
    //thread = gst_element_factory_make ("thread", "thread");
    //g_assert (thread != NULL);
    //thread2 = gst_element_factory_make ("thread", "thread2");
    //g_assert (thread2 != NULL);

    /* create a new bin to hold the elements */
    bin = gst_bin_new ("bin");
    g_assert (bin != NULL);

    /* create a disk reader */
    filesrc = gst_element_factory_make ("filesrc", "disk_source");
    g_assert (filesrc != NULL);
    g_object_set (G_OBJECT(filesrc), "location", argv[1], NULL);
    queue = gst_element_factory_make ("queue", "queue");
    queue2 = gst_element_factory_make ("queue", "queue2");

    /* and an audio sink */
    osssink = gst_element_factory_make ("osssink", "play_audio");
    g_assert (osssink != NULL);
    decode = gst_element_factory_make ("mad", "decode");
    g_assert (decode != NULL);

    /* add objects to the main bin */
    gst_bin_add_many (GST_BIN (bin), filesrc, queue, NULL);
    //gst_bin_add_many (GST_BIN (thread), decode, queue2, NULL);
    //gst_bin_add (GST_BIN (thread2), osssink);
    gst_element_link_many (filesrc, queue, decode, queue2, osssink, NULL);
    //gst_bin_add_many (GST_BIN (bin), thread, thread2, NULL);
    /* write the bin to stdout */
    gst_xml_write_file (GST_ELEMENT (bin), stdout);
    /* write the bin to a file */
    gst_xml_write_file (GST_ELEMENT (bin), fopen ("/tmp/xmlTest.gst", "w"));
    exit (0);
}
