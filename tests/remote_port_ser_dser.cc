#include <flexr>


TEST_CASE("flexr remote ports and ser&deser funcs", "[flexr_remote_ports_ser_deser]")
{
  flexr::components::ZMQPort tcpSender, tcpReceiver;
  tcpReceiver.bind(49992);
  tcpSender.connect("127.0.0.1", 49992);

  flexr::components::RtpPort rtpSender, rtpReceiver;
  rtpSender.init("127.0.0.1", -1, 49994);
  rtpReceiver.init("127.0.0.1", 49994, -1);

  uint8_t *data        = nullptr;
  uint32_t size        = 0;
  uint8_t* serData     = nullptr;
  uint32_t serDataSize = 0;
  uint32_t ts          = 0;
  bool isBlocking      = true;
  bool received        = false;


  SECTION("baic send & recv")
  {
    flexr::types::Message<uint8_t> sendMsg;
    sendMsg.setHeader("sendMsg", 0, 5000, sizeof(uint8_t));
    sendMsg.data = 44;

    // TCP
    zmq::message_t zmqMsg(&sendMsg, sizeof(sendMsg));
   tcpSender.socket.send(zmqMsg, zmq::send_flags::none);
    tcpReceiver.receiveMsg(isBlocking, data, size, ts);
    flexr::types::Message<uint8_t> *recvMsg = (flexr::types::Message<uint8_t>*)data;

    REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
    REQUIRE(recvMsg->ts == 5000);
    REQUIRE(recvMsg->dataSize == sizeof(uint8_t));
    REQUIRE(recvMsg->data == 44);

    delete data;
    data = nullptr;
    recvMsg = nullptr;

    // RTP
    rtpSender.send((uint8_t*)&sendMsg, sizeof(sendMsg), sendMsg.ts);
    rtpReceiver.receiveMsg(isBlocking, data, size, ts);

    recvMsg = (flexr::types::Message<uint8_t>*)data;
    REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
    REQUIRE(recvMsg->ts == 5000);
    REQUIRE(recvMsg->dataSize == sizeof(uint8_t));
    REQUIRE(recvMsg->data == 44);
  }


  SECTION("send & recv of encoded frame")
  {
    flexr::types::Message<uint8_t*> sendMsg;
    sendMsg.setHeader("sendMsg", 0, 5000, 250000);
    sendMsg.data = new uint8_t[250000];
    for(int i = 0; i < 100; i++) sendMsg.data[i] = i;
    flexr::utils::serializeEncodedFrame(&sendMsg, serData, serDataSize);

    // TCP
    zmq::message_t zmqMsg(serData, serDataSize);
    tcpSender.socket.send(zmqMsg, zmq::send_flags::none);
    REQUIRE(sendMsg.data == nullptr);

    received = tcpReceiver.receiveMsg(isBlocking, data, size, ts);
    flexr::types::Message<uint8_t*> *recvMsg = new flexr::types::Message<uint8_t*>;
    if(received)
    {
      flexr::utils::deserializeEncodedFrame(data, size, (void**)&recvMsg);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->dataSize == 250000);
      for(int i = 0; i < 30; i++) REQUIRE(recvMsg->data[i] == i);
    }
    received = false;
    delete recvMsg->data;
    delete recvMsg;

    // RTP
    recvMsg = new flexr::types::Message<uint8_t*>;
    rtpSender.send(serData, serDataSize, sendMsg.ts);
    received = rtpReceiver.receiveMsg(isBlocking, data, size, ts);
    if(received)
    {
      flexr::utils::deserializeEncodedFrame(data, size, (void**)&recvMsg);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->dataSize == 250000);
      for(int i = 0; i < 30; i++) REQUIRE(recvMsg->data[i] == i);
    }
  }


  SECTION("send & recv of raw frame")
  {
    flexr::types::Message<flexr::types::Frame> sendFrame, *recvFrame = new flexr::types::Message<flexr::types::Frame>;
    cv::Mat frame(1080, 1920, CV_8UC3);
    sendFrame.data.copyFromCvMat(frame);

    sendFrame.setHeader("sendRawFrame", 0, 5000, sendFrame.data.dataSize);
    flexr::utils::serializeRawFrame(&sendFrame, serData, serDataSize);
    REQUIRE(sendFrame.data.data == nullptr);

    // serialize data size = frame size + message metadata + frame metadata
    REQUIRE(serDataSize == sendFrame.dataSize + \
                           sizeof(flexr::types::Message<flexr::types::Frame>) + sizeof(flexr::types::Frame));

    zmq::message_t zmqMsg(serData, serDataSize);
    tcpSender.socket.send(zmqMsg, zmq::send_flags::none);
    received = tcpReceiver.receiveMsg(isBlocking, data, size, ts);
    if(received)
    {
      REQUIRE(size == serDataSize);
      flexr::utils::deserializeRawFrame(data, size, (void**)&recvFrame);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvFrame->tag, "sendRawFrame") == 0);
      REQUIRE(recvFrame->seq == 0);
      REQUIRE(recvFrame->ts == 5000);
      REQUIRE(recvFrame->dataSize == sendFrame.dataSize);
      REQUIRE(recvFrame->data.cols == 1920);
      REQUIRE(recvFrame->data.rows == 1080);
      recvFrame->data.release();
    }
    delete recvFrame;
  }


  SECTION("send & recv of vector")
  {
    flexr::types::Message<std::vector<int>> sendMsg, *recvMsg = new flexr::types::Message<std::vector<int>>;

    sendMsg.setHeader("sendVector", 0, 5000, sizeof(int)*10);
    sendMsg.data.resize(10);
    for(int i = 0; i < 10; i++) sendMsg.data[i] = i*i;

    flexr::utils::serializeVector<flexr::types::Message<std::vector<int>>>(&sendMsg, serData, serDataSize);
    REQUIRE(sendMsg.data.size() == 0);
    REQUIRE(serDataSize == sizeof(flexr::types::Message<std::vector<int>>) + sizeof(int)*10);

    rtpSender.send(serData, serDataSize, sendMsg.ts);
    received = rtpReceiver.receiveMsg(isBlocking, data, size, ts);
    if(received)
    {
      REQUIRE(size == serDataSize);
      flexr::utils::deserializeVector<flexr::types::Message<std::vector<int>>>(data, size, (void**)&recvMsg);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendVector") == 0);
      REQUIRE(recvMsg->seq == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->dataSize == sendMsg.dataSize);
      for(int i = 0; i < 10; i++) REQUIRE(recvMsg->data[i] == i*i);
      recvMsg->data.clear();
    }
  }
}

