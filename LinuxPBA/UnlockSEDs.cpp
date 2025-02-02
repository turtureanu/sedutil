/* C:B**************************************************************************
This software is Copyright 2014-2017 Bright Plaza Inc.
<drivetrust@drivetrust.com>

This file is part of sedutil.

sedutil is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sedutil is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sedutil.  If not, see <http://www.gnu.org/licenses/>.

* C:E********************************************************************** */
#include "UnlockSEDs.h"
#include "DtaDevGeneric.h"
#include "DtaDevOpal1.h"
#include "DtaDevOpal2.h"
#include "os.h"

#include <algorithm>
#include <dirent.h>
#include <fnmatch.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>

using namespace std;

void PrintHorizontallyCentered(string message) {
  // Get TTY dimensions
  struct winsize window;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
  int const COLS = window.ws_col;

  for (long unsigned int i = 0; i < COLS / 2 - message.length() / 2; i++) {
    cout << ' ';
  }

  cout << message;
}

uint8_t UnlockSEDs(char *password) {
  /* Loop through drives */
  char devref[25];
  int failed = 0;
  DtaDev *tempDev;
  DtaDev *d;
  DIR *dir;
  struct dirent *dirent;
  vector<string> devices;
  string tempstring;
  LOG(D4) << "Enter UnlockSEDs";
  dir = opendir("/dev");
  if (dir != NULL) {
    while ((dirent = readdir(dir)) != NULL) {
      if ((!fnmatch("sd[a-z]", dirent->d_name, 0)) ||
          (!fnmatch("nvme[0-9]", dirent->d_name, 0)) ||
          (!fnmatch("nvme[0-9][0-9]", dirent->d_name, 0))) {
        tempstring = dirent->d_name;
        devices.push_back(tempstring);
      }
    }
    closedir(dir);
  }
  std::sort(devices.begin(), devices.end());
  for (uint16_t i = 0; i < devices.size(); i++) {
    cout << "\n\n";
    snprintf(devref, 23, "/dev/%s", devices[i].c_str());
    tempDev = new DtaDevGeneric(devref);
    if (!tempDev->isPresent()) {
      break;
    }
    if ((!tempDev->isOpal1()) && (!tempDev->isOpal2())) {
      PrintHorizontallyCentered("TO SAY THAT A DRIVE'S NOT OPAL!");
      delete tempDev;
      continue;
    }
    if (tempDev->isOpal2())
      d = new DtaDevOpal2(devref);
    else
      d = new DtaDevOpal1(devref);
    delete tempDev;
    d->no_hash_passwords = false;
    failed = 0;
    if (d->Locked()) {
      if (d->MBREnabled()) {
        if (d->setMBRDone(1, password)) {
          failed = 1;
        }
      }
      if (d->setLockingRange(0, OPAL_LOCKINGSTATE::READWRITE, password)) {
        failed = 1;
      }
      /*failed ? printf("Drive %-10s %-40s is OPAL Failed  \n", devref,
         d->getModelNum()) : printf("Drive %-10s %-40s is OPAL Unlocked   \n",
         devref, d->getModelNum());*/
      if (failed) {
        cout << "\033[1;38;5;184m";
        PrintHorizontallyCentered("TO BOCK TRAFFIC!");
      } else {
        cout << "\033[1;38;5;46m";
        PrintHorizontallyCentered("TO GET TO THE OTHER SIDE!");
      }
      delete d;
    } else {
      PrintHorizontallyCentered("TO SAY THAT A DRIVE'S NOT LOCKED!");
      delete d;
    }
  }

  cout << flush;

  return 0x00;
};
