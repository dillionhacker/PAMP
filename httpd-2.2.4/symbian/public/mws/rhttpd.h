#ifndef mws_rhttpd_h
#define mws_rhttpd_h
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
#include "mws/thttpdstate.h"
#include "mws/thttpdsettings.h"


namespace Mws
{
    /**
       Defines the low-level handle interface to the Symbian server
       in whose context the Apache httpd server is run, in one or more 
       threads.
    */

    class RHttpd : public RSessionBase
        {
    public:
        typedef THttpdSettings TSettings;
        typedef THttpdState    TState;
        
        IMPORT_C RHttpd();
        
        /**
           Connects a client to the Symbian server in whose context
           the Apache httpd server is run. If the Symbian server is
           not running, it will be started.
           
           @return KErrNone is the server is successfully connected to.
        */
        IMPORT_C TInt Connect();
        
        /**
           Closes the connection to the server.
           
           NOTE: The Symbian server will exit only if there are no
                 other live connections to the server and Apache httpd
                 is not running.
        */
        IMPORT_C void Close();
        
        /**
           Returns the version of this interface.
        */
        IMPORT_C TVersion Version() const;
        
        /**
           Starts Apache httpd in a secondary thread inside the
           Symbian server.
           
           @param aSettings The settings using which Apache should
           be started. If a default constructed settings object is
           provided, then the stored settings are used. If there
           are no stored settings, then default values will be used.

           @return KErrNone  if Apache is successfully started.
                   KErrInUse if Apache is already running.
        */
        IMPORT_C TInt StartServer(const TSettings& aSettings = 
                                  TSettings()) const;
        
        /**
           Starts Apache httpd in a secondary thread inside the
           Symbian server.
           
           @param aCommandLine A command line that is provied to
           the main function of Apache.
           @param aSettings The settings using which Apache should
           be started. If a default constructed settings object is
           provided, then the stored settings are used. If there
           are no stored settings, then default values will be used.

           @return KErrNone  if Apache is successfully started.
                   KErrInUse if Apache is already running.
        */
        IMPORT_C TInt StartServer(const TDesC&     aCommandLine, 
                                  const TSettings& aSettings = 
                                  TSettings()) const;
        
        /**
           Stops Apache httpd.
           
           @param aGraceful Whether the stopping should be done
           gracefully or not. Graceful stopping means that any
           request processing activity is allowed to complete
           before Apache shuts down.
           
           @return KErrNone. Note that the return value only
           denotes that the shutdown request was successfully 
           received, not that the shutdown has taken place. You
           need to monitor the state in order to really find
           out when the shutdown has completed.
        */
        IMPORT_C TInt StopServer(TBool aGraceful = true) const;
        
        /**
           Restarts Apache httpd. That is, the httpd.conf is
           re-read.
           
           @param aGraceful Whether the restarting should be done
           gracefully or not. Graceful restarting means that any
           request processing activity is allowed to complete 
           before Apache restarts.
           
           @return KErrNone. Note that the return value only
           denotes that the restarte request was successfully 
           received, not that the restarting has taken place. 
           You need to monitor the state in order to really find
           out when the restarting has completed.
        */
        IMPORT_C TInt RestartServer(TBool aGraceful = true) const;

        /**
           Returns the current state of Apache.
           
           @param aState On successfull return contains the current
           state.
           
           @return KErrNone if successful.
         */
        IMPORT_C TInt GetState(TState& aState) const;
        
        /**
           Signals when a state-change has occurred. 
           
           @param apState Pointer to variable where the state
           will be written.
           @param aStatus TRequestStatus that will be completed
           when a state change has occurred.
           
           @return KErrNone if successful.
         */
        IMPORT_C void GetState(TPckg<TState>*  apState,
                               TRequestStatus& aStatus) const;
        
        /**
           Cancels an outstanding state change request.
           
           @return KErrNone if successful.
        */
        
        IMPORT_C TInt GetStateCancel() const;

        /**
           Stores settings to be used if no specific settings are
           specified when Apache is started.
           
           @param aSettings The settings to be stored.
        */
        IMPORT_C TInt SetSettings(const TSettings& aSettings) const;
        
        /**
           Returns the stored settings.
           
           @param aSettings Upon successful return, contains the
           stored settings.
           
           @return KErrNone if successful.
        */
        
        IMPORT_C TInt GetSettings(TSettings& aSettings) const;
        
        /**
           Returns the currently used settings.
           
           @param aSettings Upon successful return, contains the
           currently used settings. If Apache is not running, then
           the stored settings are returned.
           
           @return aSettings
        */
        IMPORT_C TInt GetCurrentSettings(TSettings& aSettings) const;
    };
}

#endif
