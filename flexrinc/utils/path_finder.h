#ifndef __FLEXR_UTILS_PATH_FINDER__
#define __FLEXR_UTILS_PATH_FINDER__

#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include "root_directory.h"

namespace flexr {
  namespace utils {

    class PathFinder {
    private:
      typedef std::string (*Finder) (const std::string& path);

      static Finder getFinder() {
        if(findRoot() != "")
          return &PathFinder::findFromRoot;
        else
          return &PathFinder::findFromBin;
      }

      static std::string const& findRoot() {
        static char const* envRoot = getenv("FLEXR_HOME");
        static char const* givenRoot = (envRoot != nullptr ? envRoot : cmakeRoot);
        static std::string foundRoot = (givenRoot != nullptr ? givenRoot : "");
        return foundRoot;
      }

      static std::string findFromRoot(const std::string& path) {
        return findRoot() + std::string("/") + path;
      }

      static std::string findFromBin(const std::string& path) {
        return "../../" + path;
      }
    public:
      static std::string find(const std::string& path) {
        static std::string(*Finder)(std::string const &) = getFinder();
        return (*Finder)(path);
      }
    };

  } // namespacet types
} // namespace flexr

#endif

