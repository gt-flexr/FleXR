#ifdef __FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>


namespace flexr
{
  namespace kernels
  {
    void EurocMonoInertialReader::LoadImages(const std::string &imgPath, const std::string &imgTsPath,
                                             std::vector<std::string> &imgFileNames,
                                             std::vector<double> &imgTs, int imgTotal)
    {
      std::ifstream fTimes;
      fTimes.open(imgTsPath.c_str());
      imgFileNames.reserve(imgTotal);
      imgTs.reserve(imgTotal);

      debug_print("Load Imgs...");
      debug_print("imgPath: %s", imgPath.c_str());
      debug_print("imgTsPath: %s", imgTsPath.c_str());
      while(!fTimes.eof())
      {
        std::string s;
        getline(fTimes,s);
        if(!s.empty())
        {
          std::stringstream ss;
          ss << s;
          imgFileNames.push_back(imgPath + "/" + ss.str() + ".png");
          double t;
          ss >> t;
          imgTs.push_back(t/1e9);
        }
      }
      debug_print("DONE");
    }


    void EurocMonoInertialReader::LoadImu(const std::string &imuPath, std::vector<double> &imuTs,
                                          std::vector<cv::Point3f> &imuAcc, std::vector<cv::Point3f> &imuGyro,
                                          int imuTotal)
    {
      std::ifstream fImu;
      fImu.open(imuPath.c_str());
      imuTs.reserve  (imuTotal);
      imuAcc.reserve (imuTotal);
      imuGyro.reserve(imuTotal);

      debug_print("Load Imus...");
      debug_print("imuPath: %s", imuPath.c_str());

      while(!fImu.eof())
      {
        std::string s;
        getline(fImu,s);
        if (s[0] == '#')
          continue;

        if(!s.empty())
        {
          std::string item;
          size_t pos = 0;
          double data[7];
          int count = 0;
          while ((pos = s.find(',')) != std::string::npos) {
            item = s.substr(0, pos);
            data[count++] = stod(item);
            s.erase(0, pos + 1);
          }
          item = s.substr(0, pos);
          data[6] = stod(item);

          imuTs.push_back(data[0]/1e9);
          imuAcc.push_back(cv::Point3f(data[4],data[5],data[6]));
          imuGyro.push_back(cv::Point3f(data[1],data[2],data[3]));
        }
      }

      debug_print("DONE");
    }


    EurocMonoInertialReader::EurocMonoInertialReader(std::string id, int fps, std::string imgPath, std::string imgTsPath,
                                                     int imgTotal, std::string imuPath, int imuTotal): FleXRKernel(id)
    {
      setName("EurocMonoInertialReader");
      freqManager.setFrequency(fps);

      portManager.registerOutPortTag("out_frame", flexr::utils::sendLocalFrameCopy, flexr::utils::serializeRawFrame);
      portManager.registerOutPortTag("out_imus",
                                     flexr::utils::sendLocalBasicCopy<EurocImusMsgType>,
                                     flexr::utils::serializeVector<EurocImusMsgType>);

      totalImus = totalImgs = firstImu = seq = 0;

      LoadImages(imgPath, imgTsPath, imgFileNames, imgTs, imgTotal);
      LoadImu(imuPath, imuTs, imuAcc, imuGyro, imuTotal);

      totalImgs = imgFileNames.size();
      totalImus = imuTs.size();
      debug_print("totalImgs: %d, totalImus: %d", totalImgs, totalImus);
      if((totalImgs<=0)||(totalImus<=0))
      {
        exit(1);
      }

      imgTsTrack.resize(totalImgs);

      while(imuTs[firstImu] <= imgTs[0]) firstImu++;
      firstImu--;
      debug_print("totalImgs %d, totalImus %d, firstImu %d", totalImgs, totalImus, firstImu);
    }


    raft::kstatus EurocMonoInertialReader::run()
    {
      EurocImgMsgType *outFrame = portManager.getOutputPlaceholder<EurocImgMsgType>("out_frame");
      EurocImusMsgType *outImus = portManager.getOutputPlaceholder<EurocImusMsgType>("out_imus");

      if(seq >= totalImgs)
      {
        debug_print("stream sequence is done");
        return raft::stop;
      }

      double st = getTsNow();

      cv::Mat img = cv::imread(imgFileNames[seq], cv::IMREAD_UNCHANGED);
      if(img.empty())
      {
        debug_print("Image load error");
        exit(1);
      }
      outFrame->data = flexr::types::Frame(img);
      outFrame->setHeader("euroc_frame", seq, imgTs[seq], outFrame->data.useAsCVMat().total()*outFrame->data.useAsCVMat().elemSize());

      if(seq > 0)
      {
        while(imuTs[firstImu] <= imgTs[seq])
        {
          outImus->data.push_back(types::ImuPoint(imuAcc[firstImu].x, imuAcc[firstImu].y, imuAcc[firstImu].z,
                                                  imuGyro[firstImu].x, imuGyro[firstImu].y, imuGyro[firstImu].z,
                                                  imuTs[firstImu]) );
          firstImu++;
        }
      }
      outImus->setHeader("euroc_imus", seq, getTsNow(), sizeof(outImus->data[0])*outImus->data.size());
      //debug_print("imuTs:%f <= imgTs:%f, firstImu:%d, ImgSeq:%d", imuTs[firstImu-1], imgTs[seq], firstImu, seq);

      seq++;

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame with Imus\t start\t{}\t end\t{}\t exe\t{}", seq-1, st, et, et-st);

      portManager.sendOutput<EurocImgMsgType>("out_frame", outFrame);
      portManager.sendOutput<EurocImusMsgType>("out_imus", outImus);

      freqManager.adjust();
      return raft::proceed;
    }


  } // namespace kernels
} // namespace flexr

#endif

