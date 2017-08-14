/* Copyright 2008 Nokia Corporation
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

#include "apr_arch_networkio.h"
#include "apr_network_io.h"
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>

extern "C"
{

APR_DECLARE(apr_status_t) symbian_socket_iap_assign(apr_socket_t* sock,
                                                    const char*   iap)
{
    apr_status_t
        rv = APR_SUCCESS;

    if (!sock->iap) {
        if (iap) {
            struct ifreq
                ifr;
            
            memset(&ifr, sizeof(ifr), 0);
            strncpy(ifr.ifr_name, iap, IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = 0;
            
            int
                size = strlen(ifr.ifr_name) + 1;
            char
                *pName = static_cast<char*>(apr_palloc(sock->pool, size));
            
            if (pName) {
                if (ioctl(sock->socketdes, SIOCSIFNAME, &ifr) == 0) {
                    strcpy(pName, ifr.ifr_name);
                    
                    sock->iap = pName;
                    
                    if (ioctl(sock->socketdes, SIOCIFSTART, &ifr) == 0) {
                        sock->iap_started = 1;
                    }
                    else
                        rv = errno;
                }
                else
                    rv = errno;
            }
            else
                rv = APR_ENOMEM;
        }
    }
    else
        rv = APR_EINVAL;

    return rv;
}

}
