/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2007 Nokia Corporation                              	  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
 */

#include "hostresolver.h"
#include <stdlib.h>
#include <es_sock.h>
#include <in_sock.h>
#include <es_enum.h>
#include <CommDbConnPref.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h> 


namespace
{
    static char*          addr_list[2];
    static struct in_addr in_addr;
    static struct hostent host;
    
    struct hostent* gethostbyname_iap(const char*    pName, 
                                      RHostResolver& resolver)
        {
        struct hostent
            *pHost = 0;
        TInt
            rc;

        TNameRecord
            record;
        TNameEntry
            entry(record);
        TPtrC8
            name8(reinterpret_cast<const TUint8*>(pName));
        TBuf<255>
            name;
        
        name.Copy(name8.Left(255));

        rc = resolver.GetByName(name, entry);
        
        if (rc == KErrNone)
            {
            record = entry();

            memset(&addr_list, 0, sizeof(addr_list));
            memset(&in_addr, 0, sizeof(in_addr));

            delete [] host.h_name;
            memset(&host, 0, sizeof(host));

            TInetAddr
                addr = TInetAddr::Cast(record.iAddr);
            
            in_addr.s_addr = htonl(addr.Address()); // IPv4
            
            addr_list[0] = reinterpret_cast<char*>(&in_addr);

            TBuf8<0x100 + 1> // See definition of THostName
                h_name8;
            
            h_name8.Copy(record.iName);

            host.h_name = new char [h_name8.Length() + 1];

            if (host.h_name)
                {
                strcpy(host.h_name, 
                       reinterpret_cast<const char*>(h_name8.PtrZ()));
                host.h_addrtype  = AF_INET;
                host.h_length    = sizeof(in_addr);
                host.h_addr_list = addr_list;
                
                pHost = &host;
                }
            }

        return pHost;
        }


    struct hostent* gethostbyname_iap(const char*  pName, 
                                      RSocketServ& ss,
                                      RConnection& connection)
        {
        struct hostent
            *pHost = 0;
        TInt
            rc;
        RHostResolver
            resolver;

        rc = resolver.Open(ss, KAfInet, KProtocolInetUdp, connection);

        if (rc == KErrNone)
            {
            pHost = gethostbyname_iap(pName, resolver);
            
            resolver.Close();
            }
        
        return pHost;
        }


    struct hostent* gethostbyname_iap(const char *pName, RSocketServ& ss)
        {
        struct hostent
            *pHost = 0;
        TInt
            rc;
        RConnection
            connection;

        rc = connection.Open(ss, KAfInet);
        
        if (rc == KErrNone)
            {
            TUint
                count;
            TCommDbConnPref
                pref;

            rc = connection.EnumerateConnections(count);
            
            if ((rc == KErrNone) && (count > 0))
                {
                TPckgBuf<TConnectionInfo> 
                    connInfo;

                rc = connection.GetConnectionInfo(1, connInfo);
                
                if (rc == KErrNone)
                    {
                    rc = connection.Attach(connInfo, 
                                           RConnection::EAttachTypeNormal);

                    if (rc == KErrNone)
                        pHost = gethostbyname_iap(pName, ss, connection);
                    }
                }
            else
                {
                pref.SetDialogPreference(ECommDbDialogPrefPrompt); 
                
                rc = connection.Start(pref);
                
                if (rc == KErrNone)
                    pHost = gethostbyname_iap(pName, ss, connection);
                }

            connection.Close();
            }
        
        return pHost;
        }
}


struct hostent* gethostbyname_iap(const char *pName)
    {
    struct hostent
        *pHost = 0;
    RSocketServ
        ss;
    
    if (ss.Connect() == KErrNone)
        {
        pHost = gethostbyname_iap(pName, ss);
        
        ss.Close();
        }
    
    return pHost;
    }


#ifdef USE_ORIGINAL_BUGGY_VERSION
struct hostent* gethostbyname_iap( const char *name )
{
	RSocketServ ss;
	RConnection conn;
	RHostResolver r;
	
	ss.Connect();
	conn.Open(ss, KAfInet);
	
	TUint c;

	TCommDbConnPref pref;
	
	if( !conn.EnumerateConnections( c ) && c > 0 )
	{
		TPckgBuf<TConnectionInfo> connInfo;
		
		conn.GetConnectionInfo( 1, connInfo );
		conn.Attach( connInfo, RConnection::EAttachTypeNormal );
	}
	else
	{
		pref.SetDialogPreference( ECommDbDialogPrefPrompt ); 
		conn.Start( pref );
	}

	r.Open( ss, KAfInet, KProtocolInetUdp, conn );

	TNameRecord record;
	TNameEntry entry(record);
	
	TPtrC8 ptr( reinterpret_cast<const TUint8*> (name) );
	TBuf<255> hostname;
	hostname.Copy(ptr.Left(255));

	TInt err = r.GetByName( hostname, entry );
	
	struct hostent retVal;

	if (err == KErrNone)
	{
		record = entry();
		
		char* addr_list[1];
		
		TBuf<64> ipAddr;
		TInetAddr addr = TInetAddr::Cast( record.iAddr );
		
		in_addr ina;
		ina.s_addr = htonl( addr.Address() ); // IPv4
		*addr_list = (char*)&ina;
		
		retVal.h_name = (char*)entry().iName.PtrZ();
		retVal.h_addr_list = addr_list;
		retVal.h_length = ipAddr.Length();
		retVal.h_addrtype = AF_INET;
	}
	
	r.Close();	
	conn.Close();
	ss.Close();

	return &retVal;
}
#endif
