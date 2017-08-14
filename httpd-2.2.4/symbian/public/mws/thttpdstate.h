#ifndef mws_thttpdstate_h
#define mws_thttpdstate_h
/* Copyright 2006 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mws/httpd.h"


namespace Mws
{
    /**
       Defines the different states of the web server.
    */
    
    enum THttpdState
        {
        /**
           The Symbian server is running, but the actual web server has not 
           been started.
        */
        EHttpdNotStarted,

        /**
           The Symbian server is running and the web server has been
           started but is not servicing requests yet.
        */
        EHttpdStarting,

        /**
           The web server is running normally.
        */
        EHttpdRunning,

        /**
           The web server is restarting as defined according to Apache
           httpd parlance. That is, the httpd.conf configuration file is
           re-read.
        */
        EHttpdRestarting,

        /**
           The web server is shutting down.
        */
        EHttpdStopping,
        
        /**
           The web server has stopped normally and the Symbian server has
           exited or is about to exit.
        */
        EHttpdStopped,

        /**
           The web server has stopped abnormally and the Symbian server 
           has exited or is about to exit.
        */
        EHttpdTerminated
        };
}

#endif
