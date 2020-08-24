#include "gst/gstbus.h"
#include <raft>
#include <gstreamer-1.0/gst/gst.h>
#include <bits/stdc++.h>

using namespace std;

class producer: public raft::kernel {
  private:
  int i = 0;

  public:
  producer(): raft::kernel() {
    output.addPort<int>("o1");
  }

  virtual raft::kstatus run() {
    while(i < 1000) {
      output["o1"].push(i++);
      return raft::proceed;
    }

    return raft::stop;
  }
};

class processor: public raft::kernel {
  public:
  processor(): raft::kernel() {
    input.addPort<int>("i1");
    output.addPort<int>("o1");
  }

  virtual raft::kstatus run() {
    output["o1"].push( input["i1"].peek<int>() );
    input["i1"].recycle();
    return raft::proceed;
  }
};

class consumer: public raft::kernel {
  public:
  GstElement *pipeline, *source, *sink, *filter, *converter;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  consumer(): raft::kernel() {
    input.addPort<int>("i1");
    gst_init(NULL, NULL);

    // create pipeline elements: gst_element_factory_make("INSTANCETYPE", "NAME")
    source = gst_element_factory_make("videotestsrc", "source");
    filter = gst_element_factory_make("vertigotv", "filter");
    converter = gst_element_factory_make("videoconvert", "converter");
    sink = gst_element_factory_make("autovideosink", "sink");
    // create an empty pipeline (bin containing other elements)
    pipeline = gst_pipeline_new("test-pipeline");
    if(!pipeline || !source || !sink || !filter || !converter) {
      g_printerr("Not all elements could be created. \n");
    }

    // build the pipeline with elements
    gst_bin_add_many(GST_BIN (pipeline), source, filter, converter, sink, NULL);

    // link the pipeline elements within the same bin:
    //                                      gst_element_link(SOURCE, DESTINATION)
    if(gst_element_link(source, filter) != TRUE) {
      g_printerr("Elements could not be linked. \n");
      gst_object_unref(pipeline);
    }

    if(gst_element_link(filter, converter) != TRUE) {
      g_printerr("Elements could not be linked. \n");
      gst_object_unref(pipeline);
    }

    if(gst_element_link(converter, sink) != TRUE) {
      g_printerr("Elements could not be linked. \n");
      gst_object_unref(pipeline);
    }
    g_object_set(source, "pattern", 0, NULL);
  }

  ~consumer() {
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].peek<int>();
    printf("[Consumer] got %d (%p) \n", in1, &in1);
    input["i1"].recycle();

    cout << "11111" << endl;
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if(ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr("Unable to set the pipeline to the playing state. \n");
      gst_object_unref(pipeline);
      return raft::stop;
    }

    cout << "22222" << endl;
    // retrieve the pipeline's bus
    //  bus: object delivering the GstMessage from pipeline elements to the
    //       application / appliction thread. The actual streaming is done in
    //       another thread than the application.
    bus = gst_element_get_bus(pipeline);
    // wait until error or EoS
    cout << "33333" << endl;
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    cout << "44444" << endl;
    if(msg != NULL) {
      GError *err;
      gchar *debug_info;

      switch(GST_MESSAGE_TYPE(msg)){
        case GST_MESSAGE_ERROR:
          gst_message_parse_error(msg, &err, &debug_info);
          g_printerr("Error received from element %s: %s \n",
              GST_OBJECT_NAME(msg->src), err->message);
          g_printerr("Debugging information: %s\n",
              debug_info ? debug_info : "none");
          g_clear_error(&err);
          g_free(debug_info);
          break;

        case GST_MESSAGE_EOS:
          g_print("End-Of_Stream reached. \n");
          break;

        default:
          g_printerr("Unexpected message received. \n");
          break;
      }
      gst_message_unref(msg);
    }

    ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if(ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr("Unable to set the pipeline to the playing state. \n");
      gst_object_unref(pipeline);
      return raft::stop;
    }

    return raft::proceed;
  }
};

int main() {
  producer a;
  processor b;
  consumer c;

  raft::map m;
  m += a >> b >> c;
  m.exe();

  return EXIT_SUCCESS;
}
