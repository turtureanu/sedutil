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

#include "GetPassPhrase.h"
#include "UnlockSEDs.h"
#include "log.h"
#include <iostream>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

/* Default to output that includes timestamps and goes to stderr*/
sedutiloutput outputFormat = sedutilNormal;

void PrintCentered(string message, int LINES, int COLS) {

  // align vertically
  // LINES / 2 - 5 for all the output (message, input box, unlock status)
  for (int i = 0; i < LINES / 2 - 5; i++) {
    cout << "\n";
  }

  for (long unsigned int i = 0; i < COLS / 2 - message.length() / 2; i++) {
    cout << ' ';
  }

  cout << "\033[31;1m" << message << "\033[0m";

  cout << flush;
}

int main(int argc, char **argv) {

  CLog::Level() = CLog::FromInt(0);
  LOG(D4) << "Legacy PBA start" << endl;

  //    system ("tput clear");

  // Get TTY dimensions
  struct winsize window;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
  int const LINES = window.ws_row;
  int const COLS = window.ws_col;

  string message = "WHY DID THE CHICKEN CROSS THE ROAD?";
  PrintCentered(message, LINES, COLS);

  cout << "\n\n";

  // Go to the position at message.length() + 2
  for (long unsigned int i = 0; i < COLS / 2 - message.length() / 2 + 2; i++) {
    cout << ' ';
  }

  cout << "\033[47m"; // ANSI sequence for white background
  for (long unsigned int i = 0; i < message.length() - 4; i++) {
    cout << ' '; // fill the background till message.length() - 2 chars
  }

  for (long unsigned int i = 0; i < message.length() - 6; i++) {
    cout << '\b'; // go back to the start of the white prompt + 2 chars
  }

  cout << "\033[1m\033[30;47m" << flush;

  // screen should now look like this:
  //     the_text_message
  //
  //       white_prompt
  // the cursor is then at `i` in wh*i*te_prompt
  // the white_prompt is 4 chars shorter than the_text_message

  string p = GetPassPhrase(message.length() - 4, true);
  cout << "\033[0m";
  UnlockSEDs((char *)p.c_str());
  if (strcmp(p.c_str(), "debug")) {
    sync();
    reboot(RB_AUTOBOOT);
  }
  return 0;
}
