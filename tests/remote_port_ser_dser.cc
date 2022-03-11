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
  bool isBlocking      = true;
  bool received        = false;

  SECTION("UDP system buffer size test")
  {
    uint32_t ts = getTsNow();
    size = 1920*1080*3 + 10000;
    data = new uint8_t[1920*1080*3 + 10000];
    for(uint32_t i = 0 ; i < 1000; i+=100) data[i] = 77;

    rtpSender.send(data, size, ts);

    uint8_t *recvData = nullptr;
    uint32_t recvSize = 0;
    rtpReceiver.receiveMsg(isBlocking, recvData, recvSize);

    REQUIRE(recvSize == size);
    for(uint32_t i = 0 ; i < 1000; i+=100) REQUIRE(recvData[i] == 77);

    delete data;
  }


  SECTION("baic send & recv without ser/dser")
  {
    flexr::types::Message<uint8_t> sendMsg;
    sendMsg.setHeader("sendMsg", 0, 5000, sizeof(uint8_t));
    sendMsg.data = 44;

    // TCP
    zmq::message_t zmqMsg(&sendMsg, sizeof(sendMsg));
    tcpSender.socket.send(zmqMsg, zmq::send_flags::none);
    tcpReceiver.receiveMsg(isBlocking, data, size);
    flexr::types::Message<uint8_t> *recvMsg = (flexr::types::Message<uint8_t>*)data;

    REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
    REQUIRE(recvMsg->seq == 0);
    REQUIRE(recvMsg->ts == 5000);
    REQUIRE(recvMsg->dataSize == sizeof(uint8_t));
    REQUIRE(recvMsg->data == 44);

    delete data;
    data = nullptr;
    recvMsg = nullptr;

    // RTP
    rtpSender.send((uint8_t*)&sendMsg, sizeof(sendMsg), sendMsg.ts);
    rtpReceiver.receiveMsg(isBlocking, data, size);

    recvMsg = (flexr::types::Message<uint8_t>*)data;
    REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
    REQUIRE(recvMsg->seq == 0);
    REQUIRE(recvMsg->ts == 5000);
    REQUIRE(recvMsg->dataSize == sizeof(uint8_t));
    REQUIRE(recvMsg->data == 44);
  }


  SECTION("send & recv of encoded frame: serializeEncodedFrame, deserializeEncodedFrame")
  {
    // Set test msg
    flexr::types::Message<uint8_t*> sendMsg;
    sendMsg.setHeader("sendMsg", 0, 5000, 250000);
    sendMsg.data = new uint8_t[250000];
    for(int i = 0; i < 100; i++) sendMsg.data[i] = i;

    // Serialize test msg: freeMsgData==true
    flexr::utils::serializeEncodedFrame(&sendMsg, serData, serDataSize, true);
    REQUIRE(sendMsg.data == nullptr);

    // Assert serialized data size
    REQUIRE(serDataSize == 250000+sizeof(flexr::types::Message<uint8_t*>));

    // TCP Test
    zmq::message_t zmqMsg(serData, serDataSize);
    tcpSender.socket.send(zmqMsg, zmq::send_flags::none);

    received = tcpReceiver.receiveMsg(isBlocking, data, size);
    REQUIRE(received == true);

    if(received)
    {
      flexr::types::Message<uint8_t*> *recvMsg = new flexr::types::Message<uint8_t*>;
      flexr::utils::deserializeEncodedFrame(data, size, (void**)&recvMsg);

      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
      REQUIRE(recvMsg->seq == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->dataSize == 250000);
      for(int i = 0; i < 100; i+=10) REQUIRE(recvMsg->data[i] == i);

      delete recvMsg->data;
      delete recvMsg;
    }
    received = false;


    // RTP Test
    rtpSender.send(serData, serDataSize, sendMsg.ts);

    received = rtpReceiver.receiveMsg(isBlocking, data, size);
    REQUIRE(received == true);

    if(received)
    {
      flexr::types::Message<uint8_t*> *recvMsg = new flexr::types::Message<uint8_t*>;
      flexr::utils::deserializeEncodedFrame(data, size, (void**)&recvMsg);

      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendMsg") == 0);
      REQUIRE(recvMsg->seq == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->dataSize == 250000);
      for(int i = 0; i < 30; i++) REQUIRE(recvMsg->data[i] == i);

      delete recvMsg->data;
      delete recvMsg;
    }
  }


  SECTION("send & recv of raw frame: serializeRawFrame, deserializeRawFrame")
  {
    flexr::types::Message<flexr::types::Frame> sendFrame;

    cv::Mat frame(1080, 1920, CV_8UC3);
    for(int i = 0; i < 100; i++) frame.at<uchar>(i, i, 0) = 100;
    sendFrame.data.copyFromCvMat(frame);

    sendFrame.setHeader("sendRawFrame", 0, 5000, sendFrame.data.dataSize);
    REQUIRE(sendFrame.dataSize == 1920*1080*3);

    flexr::utils::serializeRawFrame(&sendFrame, serData, serDataSize, false);
    REQUIRE(sendFrame.data.data != nullptr);

    delete serData;
    serData = nullptr;
    serDataSize = 0;

    flexr::utils::serializeRawFrame(&sendFrame, serData, serDataSize, true);
    REQUIRE(sendFrame.data.data == nullptr);
    REQUIRE(serDataSize == sizeof(flexr::types::Message<flexr::types::Frame>) + sendFrame.dataSize);

    // TCP
    zmq::message_t zmqMsg(serData, serDataSize);
    tcpSender.socket.send(zmqMsg, zmq::send_flags::none);
    received = tcpReceiver.receiveMsg(isBlocking, data, size);
    if(received)
    {
      REQUIRE(size == serDataSize);
      flexr::types::Message<flexr::types::Frame> *recvFrame = new flexr::types::Message<flexr::types::Frame>;
      flexr::utils::deserializeRawFrame(data, size, (void**)&recvFrame);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvFrame->tag, "sendRawFrame") == 0);
      REQUIRE(recvFrame->seq == 0);
      REQUIRE(recvFrame->dataSize == sendFrame.dataSize);
      REQUIRE(recvFrame->data.cols == 1920);
      REQUIRE(recvFrame->data.rows == 1080);
      for(int i = 0; i < 100; i+=10) REQUIRE(recvFrame->data.useAsCVMat().at<uchar>(i, i, 0) == 100);
      recvFrame->data.release();
      delete recvFrame;
    }

    // RTP
    rtpSender.send(serData, serDataSize, sendFrame.ts);
    received = rtpReceiver.receiveMsg(isBlocking, data, size);
    REQUIRE(received == true);
    if(received)
    {
      REQUIRE(size == serDataSize);
      flexr::types::Message<flexr::types::Frame> *recvFrame = new flexr::types::Message<flexr::types::Frame>;
      flexr::utils::deserializeRawFrame(data, size, (void**)&recvFrame);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvFrame->tag, "sendRawFrame") == 0);
      REQUIRE(recvFrame->seq == 0);
      REQUIRE(recvFrame->dataSize == sendFrame.dataSize);
      REQUIRE(recvFrame->data.cols == 1920);
      REQUIRE(recvFrame->data.rows == 1080);
      for(int i = 0; i < 100; i+=10) REQUIRE(recvFrame->data.useAsCVMat().at<uchar>(i, i, 0) == 100);
      recvFrame->data.release();
      delete recvFrame;
    }
  }


  SECTION("Default ser/dser with boost")
  {
    flexr::types::Message<std::vector<int>> sendMsg, *recvMsg = new flexr::types::Message<std::vector<int>>;

    sendMsg.setHeader("sendVector", 0, 5000, sizeof(int)*10);
    sendMsg.data.resize(10);
    for(int i = 0; i < 10; i++) sendMsg.data[i] = i*i;

    // defualt does not have ability to free msg.
    flexr::utils::serializeDefault<flexr::types::Message<std::vector<int>>>(&sendMsg, serData, serDataSize, false);
    REQUIRE(sendMsg.data.size() != 0);
    delete serData;
    serData = nullptr;
    serDataSize = 0;

    // defualt does not have ability to free msg.
    flexr::utils::serializeDefault<flexr::types::Message<std::vector<int>>>(&sendMsg, serData, serDataSize, true);
    REQUIRE(sendMsg.data.size() != 0);
    delete serData;
    serData = nullptr;
    serDataSize = 0;


    // vector have ability to free msg.
    flexr::utils::serializeVector<flexr::types::Message<std::vector<int>>>(&sendMsg, serData, serDataSize, false);
    REQUIRE(sendMsg.data.size() != 0);
    delete serData;
    serData = nullptr;
    serDataSize = 0;

    // vector have ability to free msg.
    flexr::utils::serializeVector<flexr::types::Message<std::vector<int>>>(&sendMsg, serData, serDataSize, true);
    REQUIRE(sendMsg.data.size() == 0);

    debug_print("serData size: %d", serDataSize);

    rtpSender.send(serData, serDataSize, sendMsg.ts);
    received = rtpReceiver.receiveMsg(isBlocking, data, size);
    REQUIRE(received == true);
    if(received)
    {
      flexr::utils::deserializeDefault<flexr::types::Message<std::vector<int>>>(data, size, (void**)&recvMsg);
      REQUIRE(data == nullptr);
      REQUIRE(strcmp(recvMsg->tag, "sendVector") == 0);
      REQUIRE(recvMsg->ts == 5000);
      REQUIRE(recvMsg->seq == 0);
      REQUIRE(recvMsg->dataSize == sendMsg.dataSize);
      for(int i = 0; i < 10; i++) REQUIRE(recvMsg->data[i] == i*i);
      recvMsg->data.clear();
    }
  }

}

