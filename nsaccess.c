/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is AOLserver Code and related documentation
 * distributed by AOL.
 * 
 * The Initial Developer of the Original Code is America Online,
 * Inc. Portions created by AOL are Copyright (C) 1999 America Online,
 * Inc. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License (the "GPL"), in which case the
 * provisions of GPL are applicable instead of those above.  If you wish
 * to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * License, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the License or the GPL.
 */

/*
 * nsaccess.c --
 *
 */

#include "ns.h"

#include "tcpd.h"

NS_RCSID("$Header$");


NS_EXPORT int Ns_ModuleVersion = 1;


/*
 * Static functions defined in this file.
 */

static Tcl_ObjCmdProc AccessObjCmd;
static int AddCmds(Tcl_Interp *interp, void *arg);


/*
 *----------------------------------------------------------------------
 *
 * Ns_ModuleInit --
 *
 *      Module entry point.  The server runs this function each time
 *      the module is loaded.  The configurable greeting is checked and
 *      a function to create the Tcl command for each interp is
 *      registered.
 *
 * Results:
 *      NS_OK or NS_ERROR.
 *
 * Side effects:
 *      Global numLoaded counter is incremented.
 *
 *----------------------------------------------------------------------
 */

int
Ns_ModuleInit(char *server, char *module)
{
    char   *path, *file;

    path = Ns_ConfigGetPath(server,module,NULL); 
    if((file = Ns_ConfigGet(path,"allow_file"))) hosts_allow_table = ns_strdup(file);
    if((file = Ns_ConfigGet(path,"deny_file"))) hosts_deny_table = ns_strdup(file);

    if (Ns_TclInitInterps(server, AddCmds, 0) != NS_OK) {
        return NS_ERROR;
    }

    return NS_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * AddCmds --
 *
 *      Register module commands for a freshly created Tcl interp.
 *
 * Results:
 *      NS_OK or NS_ERROR.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static int
AddCmds(Tcl_Interp *interp, void *arg)
{
    Tcl_CreateObjCommand(interp, "ns_access", AccessObjCmd, arg, NULL);

    return NS_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * AccessObjCmd --
 *
 *   Verifies daemon name with connection from ipaddr agains host_access
 *   database. Returns 0 if access denied.
 *
 * Results:
 *      TCL_ERROR if no name given, otherwise TCL_OK.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static int
AccessObjCmd(ClientData arg, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    int rc, cmd;
    enum commands {
        cmdCheck,
        cmdSetAllowFile,
        cmdSetDenyFile,
        cmdSetAllowSeverity,
        cmdSetDenySeverity
    };
        
    static const char *sCmd[] = {
        "check",
        "setallowfile",
        "setdenyfile",
        "setallowseverity",
        "setdenyseverity",
        0
    };

    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "command ?args?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp,objv[1],sCmd,"command",TCL_EXACT,(int *)&cmd) != TCL_OK) {
        return TCL_ERROR;
    }
    switch(cmd) {
     case cmdCheck:
        if (objc != 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "daemon_name client_ipaddr ?client_username?");
            return TCL_ERROR;
        }
        rc = hosts_ctl(Tcl_GetString(objv[2]), STRING_UNKNOWN, 
                       Tcl_GetString(objv[3]), 
                       objc > 4 ? Tcl_GetString(objv[4]) : STRING_UNKNOWN);
        Tcl_SetObjResult(interp,Tcl_NewIntObj(rc));
        break;

     case cmdSetAllowFile:
        hosts_allow_table = ns_strdup(Tcl_GetString(objv[2]));
        break;

     case cmdSetDenyFile:
        hosts_deny_table = ns_strdup(Tcl_GetString(objv[2]));
        break;

     case cmdSetAllowSeverity:
        allow_severity = atoi(Tcl_GetString(objv[2]));
        break;

     case cmdSetDenySeverity:
        deny_severity = atoi(Tcl_GetString(objv[2]));
        break;
    }
    return TCL_OK;
}
