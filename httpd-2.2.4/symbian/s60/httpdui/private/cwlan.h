#ifndef cwlan_h
#define cwlan_h
/* Copyright 2008 Nokia Corporation
 *
 * Licensed under the GNU General Public License, version 2 or
 * the Apache License, version 2.0, depending on the component
 * of which this file is part of.
 *
 * Symbian port of MySQL       : GNU General Public License, version 2
 * Symbian port of Apache httpd: Apache License, version 2.0
 * PAMP                        : Apache License, version 2.0
 * Raccoon                     : Apache License, version 2.0
 */

/*
 * NOTE: This is general purpose stuff and is used by the UIs of MySQL,
 * NOTE: httpd, PAMP and Raccoon. As such it should be in some common DLL.
 * NOTE: However, currently there is no appropriate DLL, so for the
 * NOTE: time being it is simply copied around. A modification made
 * NOTE: in one place should be duplicated elsewhere.
 */

#include <e32base.h>
#include <es_sock.h>
#include <es_enum.h>
#include <in_sock.h>


NONSHARABLE_CLASS(CWLAN) : public ::CBase
    {
public:
    NONSHARABLE_CLASS(TConnectionStatus)
        {
    public:
        TConnectionStatus(const ::TConnectionInfo& aInfo,
                          const ::TNifProgress&    aProgress) :
            iInfo(aInfo),
            iProgress(aProgress)
            {}
        
        ::TConnectionInfo iInfo;
        ::TNifProgress    iProgress;
        };


    NONSHARABLE_CLASS(MObserver)
        {
    public:
        virtual void UpdateL(CWLAN*                   ipSubject, 
                             const TConnectionStatus& aStatus) = 0;
        };


    ~CWLAN();
    
    static CWLAN* NewL();
    static CWLAN* NewLC();
    
    void  ActivateL();
    void  Close();
    
    TBool ActiveL() const;
    TBool ActiveL(TConnectionInfo& aInfo) const;
    
    TBool ActivatedL() const;

    void  AttachL(MObserver* apObserver);
    void  Detach (MObserver* apObserver);

    TBool Attached(MObserver* apObserver) const;


    static void  GetIpAddressesL(RSocketServ&           aServer,
                                 const TConnectionInfo& aInfo,
                                 RArray<TInetAddr>&     aAddresses);
    void         GetIpAddressesL(const TConnectionInfo& aInfo,
                                 RArray<TInetAddr>&     aAddresses);
    // aText: TBuf<KCommsDbSvrDefaultTextFieldLength>
    TBool        GetIpAddressL  (const TConnectionInfo& aInfo, 
                                 TDes&                  aText); 
    // aName: TBuf<39>, see TInetAddr::Output()
    void         GetApNameL     (const TUint32          aIapId,
                                 TDes&                  aName);
    
private:
    CWLAN();
    
    void ConstructL();
    
public:
    class CImp;
    friend class CImp;
    
private:
    CImp* ipImp;
    };

#endif
