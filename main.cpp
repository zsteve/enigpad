/* Enigma Machine Simulator v1
(C) Stephen Zhang / 0x4c.phatcode.net
made for Joshua's birthday 2013

This application is OPEN SAUCE.
This means you can put tomato sauce on it.
*/


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "resource.h"

HINSTANCE hInstance;

typedef unsigned char uchar;

int r1[]={3,5,1,7,4,9,8,2,0,6};
int r2[]={1,3,8,6,2,5,9,4,0,7};
int r3[]={7,4,3,1,8,2,9,6,7,0};

typedef struct RotorState{
    int rs1, rs2, rs3;
};

class Rotor
{
    /* ROTOR Class **
    Reason : Enigma "rotor" simulator
    Description : Simulates the enigma machine's random
                  number rotors. Currently uses 3 rotors
    */

    int rc1, rc2, rc3;
    public:
    Rotor()
    {
        // Constructor
        rc1=rc2=rc3=0;
    }
    void SetInitialState(int r1, int r2, int r3)
    {
        rc1=r1;
        rc2=r2;
        rc3=r3;
    }
    RotorState GetInitialState()
    {
        RotorState r;
        r.rs1=rc1;
        r.rs2=rc2;
        r.rs3=rc3;
        return r;
    }
    void ResetRotors(RotorState &rs)
    {
        rc1=rs.rs1;
        rc2=rs.rs2;
        rc3=rs.rs3;
    }
    void CycleRotors()
    {
        if(++rc1>sizeof(r1))
        {
            rc1=0;
            if(++rc2>sizeof(r2))
            {
                rc2=0;
                if(++rc3>sizeof(r3))
                {
                    rc3=0;
                }
            }
        }
    }
    int GetRotorsSum()
    {
        int sum=0;
        sum+=(r1[rc1]+r2[rc2]+r3[rc3]);
        CycleRotors();
        return sum;
    }
};

class Edit{
    HWND hEdit;
    char* tHandle;
    public:
    Edit(){
        hEdit=NULL;
        tHandle=NULL;
    }
    int SetHwnd(HWND hwndNew)
    {
            hEdit=hwndNew;
    }
    void SetText(char* text)
    {
        if(tHandle)
        {
            free(tHandle);
            tHandle=NULL;
        }
        tHandle=(char*)malloc(strlen(text)+1);
        strcpy(tHandle, text);
        SetWindowText(hEdit, tHandle);
        return;
    }
    char* GetText()
    {
        int l;
        if(tHandle)
        {
            free(tHandle);
            tHandle=NULL;
        }
        l=GetWindowTextLength(hEdit);
        tHandle=(char*)malloc(l+1);
        ZeroMemory(tHandle, l);
        GetWindowText(hEdit, tHandle, l+1);
        return tHandle;
    }
};

class Encrypt
{
    /* Encryption class
    Reason : Encryption of chars
    Description : The actual engine of encrypting / decrypting
                  the text. NOTE *** : Only works for alphabet and digits.
    */
    char* CleanMessage(char* in, bool removespace)
    {
        char* out=(char*)malloc(strlen(in)+1);
        ZeroMemory(out, strlen(in)+1);
        char *final=out;
        for(; *in; out++, in++)
        {
            if(IsCharAlphaNumericA(*in))
            {
                // Char is alphanumeric (A-Z, 0-9)
                *out=*in;
                if(!isupper(*out))
                    tolower(*out);
            }
            if(!removespace && *in==' ')
            {
                // Char is space, and removespace is FALSE
                *out=*in;
            }
            else
            {
                *out=*in;
            }
        }
        return final;
    }
    char ShiftChar(char in, int shiftnum)
    {
        if(isalpha(in))
        {
            in=tolower(in);
            for(int i=0; i<shiftnum; i++)
            {
                in++;
                if(in>'z')
                    in='a';
            }
            return in;
        }
        else if(isdigit(in))
        {
            for(int i=0; i<shiftnum; i++)
            {
                in++;
                if(in>'9')
                    in='0';
            }
            return in;
        }
        return in;
    }
    char UnshiftChar(char in, int shiftnum)
    {
        if(isalpha(in))
        {
            for(int i=0; i<shiftnum; i++)
            {
                in--;
                if(in<'a')
                    in='z';
            }
            return in;
        }
        else if(isdigit(in))
        {
            for(int i=0; i<shiftnum; i++)
            {
                in--;
                if(in<'0')
                    in='9';
            }
            return in;
        }
        return in;
    }
    public:
    void EncryptText(uchar* txt, Rotor &rotor)
    {
        uchar* c=txt;
        char* p;
        uchar i;
        int j=strlen((char*)txt);
        p=CleanMessage((char*)txt, FALSE);
        strcpy((char*)c, p);
        for(; j; j--, c++)
        {
            i=rotor.GetRotorsSum();
            *c=ShiftChar(*c, i);
            rotor.CycleRotors();
        }
    }

    void DecryptText(uchar* txt, Rotor &rotor)
    {
        uchar* c=txt;
        char* p;
        uchar i;
        int j=strlen((char*)txt);
        p=CleanMessage((char*)txt, FALSE);
        strcpy((char*)c, p);
        for(; j; j--, c++)
        {
            i=rotor.GetRotorsSum();
            *c=UnshiftChar(*c, i);
            rotor.CycleRotors();
        }
    }
};
Rotor rotr;
RotorState rstate;
Edit eb1;
Edit eb2;
Encrypt encob;


BOOL CALLBACK SetRotorProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RotorState rst;
    switch(uMsg)
    {
        case WM_INITDIALOG:
                rst=rotr.GetInitialState();
                SetDlgItemInt(hWnd, IDC_R1, rst.rs1,0);
                SetDlgItemInt(hWnd, IDC_R2, rst.rs2,0);
                SetDlgItemInt(hWnd, IDC_R3, rst.rs3,0);
            return TRUE;

        case WM_CLOSE:
            EndDialog(hWnd, 0);
            return TRUE;
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_ROK:
                    rst.rs1=GetDlgItemInt(hWnd, IDC_R1, 0, 0);
                    rst.rs2=GetDlgItemInt(hWnd, IDC_R2, 0, 0);
                    rst.rs3=GetDlgItemInt(hWnd, IDC_R3, 0, 0);
                    rotr.SetInitialState(rst.rs1, rst.rs2, rst.rs3);
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                break;
                case ID_RQ:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            }
    }

    return FALSE;
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        int wid, len;
        case WM_INITDIALOG:
            union sdim{
                unsigned long big;
                struct{
                    unsigned short hi;
                    unsigned short lo;
                }smll;
            }screen_dimensions;
            eb1.SetHwnd(GetDlgItem(hWnd, IDC_EDIT1));
            eb2.SetHwnd(GetDlgItem(hWnd, IDC_EDIT2));
            screen_dimensions.smll.hi=640  ;
            screen_dimensions.smll.lo=480;
            SendMessage(hWnd, WM_SIZE, 0, screen_dimensions.big);
            MoveWindow(hWnd, 200, 200, screen_dimensions.smll.hi, screen_dimensions.smll.lo, TRUE);
            rstate.rs1=rstate.rs2=rstate.rs3=0;
            return TRUE;

        case WM_CLOSE:
            EndDialog(hWnd, 0);
            return TRUE;
            break;
        case WM_SIZE:
            // adjust size of IDC_MAINEDIT
                wid=LOWORD(lParam);
                len=HIWORD(lParam);
                MoveWindow(GetDlgItem(hWnd, IDC_EDIT1), 2,2, wid-2, len/2-1, 1);
                MoveWindow(GetDlgItem(hWnd, IDC_EDIT2), 2,len/2, wid-2, len/2-1, 1);

                break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_QUIT1:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                case IDM_ENCRYPT1:
                    {
                        uchar *txt;
                        txt=(uchar*)eb1.GetText();
                        encob.EncryptText(txt, rotr);
                        eb2.SetText((char*)txt);
                        rotr.ResetRotors(rstate);
                    }
                    break;
                case IDM_DECRYPT1:
                    {
                        uchar *txt;
                        txt=(uchar*)eb2.GetText();
                        encob.DecryptText(txt, rotr);
                        eb1.SetText((char*)txt);
                        rotr.ResetRotors(rstate);
                    }
                    break;
                case IDM_SET_INITIAL_ROTOR_SETTING1:
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_SETROTOR), NULL, (DLGPROC)SetRotorProc);
            }
    }

    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInstance = hInstance_;

    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DialogProc);
}
