/* Copyright 2000-2004 The Apache Software Foundation
 * Partly Copyright 2006 Nokia Corporation. All rights reserved
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

/*
 * The current Symbian port is based on the Unix version. However, as 
 * Symbian has the notion of drives, some modifications are needed to
 * accomodate for that.
 *
 * This file is copied from ../unix/filepath.c
 */

#include "apr.h"
#include "apr_private.h"
#include "apr_arch_file_io.h"
#include "apr_file_io.h"
#include "apr_strings.h"


namespace
{
    bool IS_FNCHAR(char c)
    {
        bool
            rc = true;
        
        switch (c)
            {
            case ':':
            case '<':
            case '>':
            case '/':
            case '\\':
            case '|':
            case '*':
            case '?':
                rc = false;
                break;

            default:
                break;
            }
        
        return rc;
    }


    bool IS_DCHAR(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }


   /* If the user passes APR_FILEPATH_TRUENAME to either
    * apr_filepath_root or apr_filepath_merge, this fn determines
    * that the root really exists.  It's expensive, wouldn't want
    * to do this too frequenly.
    */
    apr_status_t filepath_root_test(const char *path, apr_pool_t *p)
    {
        // This is not ok as the validity of the path is not checked.
        // However, apr_stat() or stat() do _not_ actually check that 
        // a path like "X:/." exists but return success every time,
        // so they can't be used.
        //
        // To really check for the validity of the drive would require
        // 
        //   1) RFs hFs; hFs.Connect();
        //   2) TDriveList list; hFs.DriveList(list);
        //   3) hFs.Close();
        //   4) Check that the asked for drive is marked as being 
        //      present.
        //
        // As I really don't want to do this so we'll just check that 
        // the drive is within the generally acceptable range. 
        //
        // Apache behaves ok with this behaviour and it is an issue 
        // only if httpd.conf contains, for instance, an alias 
        // to a non-existing drive.

        return IS_DCHAR(*path) ? APR_SUCCESS : APR_EBADPATH;
    }
    
    
    /* The apr_filepath_merge wants to canonicalize the cwd to the 
     * addpath if the user passes NULL as the old root path (this
     * isn't true of an empty string "", which won't be concatenated.
     *
     * But we need to figure out what the cwd of a given volume is,
     * when the user passes D:foo.  This fn will determine D:'s cwd.
     *
     * If flags includes the bit APR_FILEPATH_NATIVE, the path returned
     * is in the os-native format.
     */
    apr_status_t filepath_drive_get(char **rootpath, char drive, 
                                    apr_int32_t flags, apr_pool_t *p)
    {
        apr_status_t
            rc = APR_EBADPATH;
        char
            path[APR_PATH_MAX];

        getcwd(path, sizeof(path));

        // Symbian does not have the notion of drive specific current 
        // paths. We'll just fail if the path asked for is not on the
        // current drive.

        if (path[0] == drive) {
            if (!(flags & APR_FILEPATH_NATIVE)) {
                // Convert '/' to '\\'.

                for (*rootpath = path; **rootpath; ++*rootpath) {
                    if (**rootpath == '\\')
                        **rootpath = '/';
                }
            }
            
            *rootpath = apr_pstrdup(p, path);

            rc = APR_SUCCESS;
        }

        return rc;
    }

}


extern "C"
{

/* Any OS that requires/refuses trailing slashes should be dealt with here.
 */
APR_DECLARE(apr_status_t) apr_filepath_get(char**      defpath, 
                                           apr_int32_t flags,
                                           apr_pool_t* p)
{
    char 
        path[APR_PATH_MAX];

    if (!getcwd(path, sizeof(path))) {
        if (errno == ERANGE)
            return APR_ENAMETOOLONG;
        else
            return errno;
    }
    
    *defpath = apr_pstrdup(p, path);

    return APR_SUCCESS;
}


/* Any OS that requires/refuses trailing slashes should be dealt with here
 */
APR_DECLARE(apr_status_t) apr_filepath_set(const char *path, apr_pool_t *p)
{
    if (chdir(path) != 0)
        return errno;

    return APR_SUCCESS;
}


// FIXME: A fairly straight copy from ../winnt/filepath.c.
// FIXME: Obvious candidate for some cleanup.
APR_DECLARE(apr_status_t) apr_filepath_root(const char** rootpath, 
                                            const char** inpath, 
                                            apr_int32_t  flags,
                                            apr_pool_t*  p)
{
    const char 
        *testpath = *inpath;
    char *newpath;

    char separator[2];

    separator[0] = (flags & APR_FILEPATH_NATIVE) ? '\\' : '/';
    separator[1] = 0;

    if (testpath[0] == '/' || testpath[0] == '\\') {
        /* Left with a path of '/', what drive are we asking about? 
         */
        *inpath = testpath + 1;
        newpath = static_cast<char*>(apr_palloc(p, 2));

        if (flags & APR_FILEPATH_TRUENAME)
            newpath[0] = separator[0];
        else
            newpath[0] = testpath[0];

        newpath[1] = '\0';
        *rootpath = newpath;

        return APR_EINCOMPLETE;
    }

    /* Evaluate path of 'd:[/]' */
    if (IS_FNCHAR(*testpath) && testpath[1] == ':') 
    {
        apr_status_t rv;
        /* Validate that D:\ drive exists, test must be rooted
         * Note that posix/win32 insists a drive letter is upper case,
         * so who are we to argue with a 'feature'.
         * It is a safe fold, since only A-Z is legal, and has no
         * side effects of legal mis-mapped non-us-ascii codes.
         */
        newpath = static_cast<char*>(apr_palloc(p, 4));

        newpath[0] = testpath[0];
        newpath[1] = testpath[1];
        newpath[2] = separator[0];
        newpath[3] = '\0';

        if (flags & APR_FILEPATH_TRUENAME) {
            newpath[0] = apr_toupper(newpath[0]);

            rv = filepath_root_test(newpath, p);

            if (rv)
                return rv;
        }
        /* Just give back the root the user handed to us.
         */
        if (testpath[2] != '/' && testpath[2] != '\\') {
            newpath[2] = '\0';
            *rootpath = newpath;
            *inpath = testpath + 2;

            return APR_EINCOMPLETE;
        }

        /* strip off remaining slashes that designate the root,
         * give the caller back their original choice of slash
         * unless this is TRUENAME'ed
         */
        *inpath = testpath + 3;

        while (**inpath == '/' || **inpath == '\\')
            ++*inpath;

        if (!(flags & APR_FILEPATH_TRUENAME))
            newpath[2] = testpath[2];

        *rootpath = newpath;

        return APR_SUCCESS;
    }

    /* Nothing interesting */
    return APR_ERELATIVE;
}


// FIXME: A fairly straight copy from ../winnt/filepath.c.
// FIXME: Obvious candidate for some cleanup.
APR_DECLARE(apr_status_t) apr_filepath_merge(char**      newpath, 
                                             const char* basepath, 
                                             const char* addpath, 
                                             apr_int32_t flags,
                                             apr_pool_t* p)
{
    char path[APR_PATH_MAX]; /* isn't null term */
    const char *baseroot = NULL;
    const char *addroot;
    apr_size_t rootlen; /* the length of the root portion of path, d:/ is 3 */
    apr_size_t baselen; /* the length of basepath (excluding baseroot) */
    apr_size_t keptlen; /* the length of the retained basepath (incl root) */
    apr_size_t pathlen; /* the length of the result path */
    apr_size_t segend;  /* the end of the current segment */
    apr_size_t seglen;  /* the length of the segment (excl trailing chars) */
    apr_status_t basetype = 0; /* from parsing the basepath's baseroot */
    apr_status_t addtype;      /* from parsing the addpath's addroot */
    apr_status_t rv;
    
    /* Treat null as an empty path, otherwise split addroot from the addpath
     */
    if (!addpath) {
        addpath = addroot = "";
        addtype = APR_ERELATIVE;
    }
    else {
        /* This call _should_ test the path
         */
        addtype = apr_filepath_root(&addroot, &addpath, 
                                    APR_FILEPATH_TRUENAME
                                    | (flags & APR_FILEPATH_NATIVE),
                                    p);
        if (addtype == APR_SUCCESS) {
            addtype = APR_EABSOLUTE;
        }
        else if (addtype == APR_ERELATIVE) {
            addroot = "";
        }
        else if (addtype != APR_EINCOMPLETE) {
            /* apr_filepath_root was incomprehensible so fail already
             */
            return addtype;
        }
    }

    /* If addpath is (even partially) rooted, then basepath is
     * unused.  Ths violates any APR_FILEPATH_SECUREROOTTEST 
     * and APR_FILEPATH_NOTABSOLUTE flags specified.
     */
    if (addtype == APR_EABSOLUTE || addtype == APR_EINCOMPLETE)
    {
        if (flags & APR_FILEPATH_SECUREROOTTEST)
            return APR_EABOVEROOT;
        if (flags & APR_FILEPATH_NOTABSOLUTE)
            return addtype;
    }

    /* Optimized tests before we query the current working path
     */
    if (!basepath) {

        /* If APR_FILEPATH_NOTABOVEROOT wasn't specified,
         * we won't test the root again, it's ignored.
         * Waste no CPU retrieving the working path.
         */
        if (addtype == APR_EABSOLUTE && !(flags & APR_FILEPATH_NOTABOVEROOT)) {
            basepath = baseroot = "";
            basetype = APR_ERELATIVE;
        }

        /* If APR_FILEPATH_NOTABSOLUTE is specified, the caller 
         * requires an absolutely relative result, So do not retrieve 
         * the working path.
         */
        if (addtype == APR_ERELATIVE && (flags & APR_FILEPATH_NOTABSOLUTE)) {
            basepath = baseroot = "";
            basetype = APR_ERELATIVE;
        }
    }

    if (!basepath) 
    {
        /* Start with the current working path.  This is bass akwards,
         * but required since the compiler (at least vc) doesn't like
         * passing the address of a char const* for a char** arg.
         * We must grab the current path of the designated drive 
         * if addroot is given in drive-relative form (e.g. d:foo)
         */
        char *getpath;

        if (addtype == APR_EINCOMPLETE && addroot[1] == ':')
            rv = filepath_drive_get(&getpath, addroot[0], flags, p);
        else
            rv = apr_filepath_get(&getpath, flags, p);
        
        if (rv != APR_SUCCESS)
            return rv;
        
        basepath = getpath;
    }

    if (!baseroot) {
        /* This call should _not_ test the path
         */
        basetype = apr_filepath_root(&baseroot, &basepath,
                                     (flags & APR_FILEPATH_NATIVE), p);
        if (basetype == APR_SUCCESS) {
            basetype = APR_EABSOLUTE;
        }
        else if (basetype == APR_ERELATIVE) {
            baseroot = "";
        }
        else if (basetype != APR_EINCOMPLETE) {
            /* apr_filepath_root was incomprehensible so fail already
             */
            return basetype;
        }
    }
    baselen = strlen(basepath);

    /* If APR_FILEPATH_NOTABSOLUTE is specified, the caller 
     * requires an absolutely relative result.  If the given 
     * basepath is not relative then fail.
     */
    if ((flags & APR_FILEPATH_NOTABSOLUTE) && basetype != APR_ERELATIVE)
        return basetype;

    /* The Win32 nightmare on unc street... start combining for
     * many possible root combinations.
     */
    if (addtype == APR_EABSOLUTE)
    {
        /* Ignore the given root path, and start with the addroot
         */
        if ((flags & APR_FILEPATH_NOTABOVEROOT) 
                && strncmp(baseroot, addroot, strlen(baseroot)))
            return APR_EABOVEROOT;
        keptlen = 0;
        rootlen = pathlen = strlen(addroot);
        memcpy(path, addroot, pathlen);
    }
    else if (addtype == APR_EINCOMPLETE)
    {
        /* There are two types of incomplete paths, 
         *     drives without rooted paths  (d: as in d:foo), 
         *     and simple roots             (/ as in /foo).
         * Deal with these in significantly different manners...
         */
        if ((addroot[0] == '/' || addroot[0] == '\\') && !addroot[1]) 
        {
            /* Bring together the drive or UNC root from the baseroot
             * if the addpath is a simple root and basepath is rooted,
             * otherwise disregard the basepath entirely.
             */
            if (basetype != APR_EABSOLUTE && 
                (flags & APR_FILEPATH_NOTRELATIVE))
                return basetype;
            
            if (basetype != APR_ERELATIVE) {
                keptlen = rootlen = pathlen = strlen(baseroot);
                memcpy(path, baseroot, pathlen);
            }
            else {
                if (flags & APR_FILEPATH_NOTABOVEROOT)
                    return APR_EABOVEROOT;
                
                keptlen = 0;
                rootlen = pathlen = strlen(addroot);
                memcpy(path, addroot, pathlen);
            }
        }
        else if (addroot[0] && addroot[1] == ':' && !addroot[2]) 
        {
            /* If the addroot is a drive (without a volume root)
             * use the basepath _if_ it matches this drive letter!
             * Otherwise we must discard the basepath.
             */
            if (addroot[0] == baseroot[0] && baseroot[1] == ':') {
                /* Base the result path on the basepath
                 */
                if (basetype != APR_EABSOLUTE && 
                    (flags & APR_FILEPATH_NOTRELATIVE))
                    return basetype;
                
                rootlen = strlen(baseroot);
                keptlen = pathlen = rootlen + baselen;

                if (keptlen >= sizeof(path))
                    return APR_ENAMETOOLONG;
                
                memcpy(path, baseroot, rootlen);
                memcpy(path + rootlen, basepath, baselen);
            } 
            else {
                if (flags & APR_FILEPATH_NOTRELATIVE)
                    return addtype;
                
                if (flags & APR_FILEPATH_NOTABOVEROOT)
                    return APR_EABOVEROOT;
                
                keptlen = 0;
                rootlen = pathlen = strlen(addroot);
                memcpy(path, addroot, pathlen);
            }
        }
        else {
            /* Now this is unexpected, we aren't aware of any other
             * incomplete path forms!  Fail now.
             */
            return APR_EBADPATH;
        }
    }
    else { /* addtype == APR_ERELATIVE */
        /* If both paths are relative, fail early
         */
        if (basetype != APR_EABSOLUTE && (flags & APR_FILEPATH_NOTRELATIVE))
            return basetype;

        /* Base the result path on the basepath
         */
        rootlen = strlen(baseroot);
        keptlen = pathlen = rootlen + baselen;
        
        if (keptlen >= sizeof(path))
            return APR_ENAMETOOLONG;
        
        memcpy(path, baseroot, rootlen);
        memcpy(path + rootlen, basepath, baselen);
    }

    /* '/' terminate the given root path unless it's already terminated
     * or is an incomplete drive root.  Correct the trailing slash unless
     * we have an incomplete UNC path still to fix.
     */
    if (pathlen && path[pathlen - 1] != ':') {
        if (path[pathlen - 1] != '/' && path[pathlen - 1] != '\\') {
            if (pathlen + 1 >= sizeof(path))
                return APR_ENAMETOOLONG;
        
            path[pathlen++] = ((flags & APR_FILEPATH_NATIVE) ? '\\' : '/');
        }
    /*  XXX: wrong, but gotta figure out what I intended;
     *  else if (!fixunc)
     *      path[pathlen++] = ((flags & APR_FILEPATH_NATIVE) ? '\\' : '/');
     */
    }

    while (*addpath) 
    {
        /* Parse each segment, find the closing '/' 
         */
        seglen = 0;
        while (addpath[seglen] && addpath[seglen] != '/'
                               && addpath[seglen] != '\\')
            ++seglen;

        /* Truncate all trailing spaces and all but the first two dots */
        segend = seglen;
        while (seglen && (addpath[seglen - 1] == ' ' 
                       || addpath[seglen - 1] == '.')) {
            if (seglen > 2 || addpath[seglen - 1] != '.' || addpath[0] != '.')
                --seglen;
            else
                break;
        }

        if (seglen == 0 || (seglen == 1 && addpath[0] == '.')) 
        {
            /* NOTE: win32 _hates_ '/ /' and '/. /' (yes, with spaces in there)
             * so eliminate all preconceptions that it is valid.
             */
            if (seglen < segend)
                return APR_EBADPATH;

            /* Otherwise, this is a noop segment (/ or ./) so ignore it 
             */
        }
        else if (seglen == 2 && addpath[0] == '.' && addpath[1] == '.') 
        {
            /* NOTE: win32 _hates_ '/.. /' (yes, with a space in there)
             * and '/..../', some functions treat it as ".", and some 
             * fail! Eliminate all preconceptions that they are valid.
             */
            if (seglen < segend && (seglen != 3 || addpath[2] != '.'))
                return APR_EBADPATH;

            /* backpath (../) when an absolute path is given */
            if (rootlen && (pathlen <= rootlen)) 
            {
                /* Attempt to move above root.  Always die if the 
                 * APR_FILEPATH_SECUREROOTTEST flag is specified.
                 */
                if (flags & APR_FILEPATH_SECUREROOTTEST)
                    return APR_EABOVEROOT;
                
                /* Otherwise this is simply a noop, above root is root.
                 */
            }
            else if (pathlen == 0 ||
                     (pathlen >= 3 && (pathlen == 3
                                    || path[pathlen - 4] == ':')
                                   &&  path[pathlen - 3] == '.' 
                                   &&  path[pathlen - 2] == '.' 
                                   && (path[pathlen - 1] == '/' 
                                    || path[pathlen - 1] == '\\')))
            {
                /* Path is already backpathed or empty, if the
                 * APR_FILEPATH_SECUREROOTTEST.was given die now.
                 */
                if (flags & APR_FILEPATH_SECUREROOTTEST)
                    return APR_EABOVEROOT;

                /* Otherwise append another backpath.
                 */
                if (pathlen + 3 >= sizeof(path))
                    return APR_ENAMETOOLONG;
                memcpy(path + pathlen, ((flags & APR_FILEPATH_NATIVE) 
                                          ? "..\\" : "../"), 3);
                pathlen += 3;
                /* The 'root' part of this path now includes the ../ path,
                 * because that backpath will not be parsed by the truename
                 * code below.
                 */
                keptlen = pathlen;
            }
            else 
            {
                /* otherwise crop the prior segment 
                 */
                do {
                    --pathlen;
                } while (pathlen && path[pathlen - 1] != '/'
                                 && path[pathlen - 1] != '\\');

                /* Now test if we are above where we started and back up
                 * the keptlen offset to reflect the added/altered path.
                 */
                if (pathlen < keptlen) 
                {
                    if (flags & APR_FILEPATH_SECUREROOTTEST)
                        return APR_EABOVEROOT;
                    keptlen = pathlen;
                }
            }
        }
        else /* not empty or dots */
        {
            /* An actual segment, append it to the destination path
             */
            apr_size_t i = (addpath[segend] != '\0');
            if (pathlen + seglen + i >= sizeof(path))
                return APR_ENAMETOOLONG;
            memcpy(path + pathlen, addpath, seglen + i);
            if (i)
                path[pathlen + seglen] = ((flags & APR_FILEPATH_NATIVE) 
                                          ? '\\' : '/');
            pathlen += seglen + i;
        }

        /* Skip over trailing slash to the next segment
         */
        if (addpath[segend])
            ++segend;

        addpath += segend;
    }
    
    /* keptlen will be the baselen unless the addpath contained
     * backpath elements.  If so, and APR_FILEPATH_NOTABOVEROOT
     * is specified (APR_FILEPATH_SECUREROOTTEST was caught above),
     * compare the string beyond the root to assure the result path 
     * is still within given basepath.  Note that the root path 
     * segment is thoroughly tested prior to path parsing.
     */
    if (flags & APR_FILEPATH_NOTABOVEROOT) {
        if (memcmp(basepath, path + rootlen, baselen))
            return APR_EABOVEROOT;

        /* Ahem... if we weren't given a trailing slash on the basepath,
         * we better be sure that /foo wasn't replaced with /foobar!
         */
        if (basepath[baselen - 1] != '/' && basepath[baselen - 1] != '\\'
                && path[rootlen + baselen] && path[rootlen + baselen] != '/' 
                                           && path[rootlen + baselen] != '\\')
            return APR_EABOVEROOT;
    }

    if (addpath && (flags & APR_FILEPATH_TRUENAME)) {
        /* We can always skip the root, it's already true-named. */
        if (rootlen > keptlen)
            keptlen = rootlen;
        if ((path[keptlen] == '/') || (path[keptlen] == '\\')) {
            /* By rights, keptlen may grown longer than pathlen.
             * we wont' use it again (in that case) so we don't care.
             */
            ++keptlen;
        }
        /* Go through all the new segments */
        while (keptlen < pathlen) {
            apr_finfo_t finfo;
            char saveslash = 0;
            seglen = 0;
            /* find any slash and set it aside for a minute. */
            for (seglen = 0; keptlen + seglen < pathlen; ++seglen) {
                if ((path[keptlen + seglen] == '/')  ||
                    (path[keptlen + seglen] == '\\')) {
                    saveslash = path[keptlen + seglen];
                    break;
                }
            }

            /* Null term for stat! */
            path[keptlen + seglen] = '\0';
            
            rv = apr_stat(&finfo, path, APR_FINFO_TYPE, p);
            
            if (rv == APR_SUCCESS) {
                /* That's it, the rest is path info. 
                 * I don't know how we aught to handle this.  Should
                 * we define a new error to indicate 'more info'?
                 * Should we split out the rest of the path?
                 */
                if ((finfo.filetype != APR_DIR) && 
                    (finfo.filetype != APR_LNK) && saveslash) 
                    rv = APR_ENOTDIR;
            }

            /* put back the '/' */
            if (saveslash) {
                path[keptlen + seglen] = saveslash;
                ++seglen;
            }
            keptlen += seglen;

            if (rv != APR_SUCCESS) {
                if (APR_STATUS_IS_ENOENT(rv))
                    break;
                if (APR_STATUS_IS_EPATHWILD(rv))
                    /* This path included wildcards.  The path elements
                     * that did not contain wildcards are canonicalized,
                     * so we will return the path, although later elements
                     * don't necessarily exist, and aren't canonical.
                     */
                    break;
                else if (APR_STATUS_IS_ENOTDIR(rv))
                    /* This is a little more serious, we just added a name
                     * onto a filename (think http's PATH_INFO)
                     * If the caller is foolish enough to do this, we expect
                     * the've already canonicalized the root) that they knew
                     * what they are doing :(
                     */
                    break;
                else
                    return rv;
            }
        }
    }

    *newpath = static_cast<char*>(apr_pmemdup(p, path, pathlen + 1));
    (*newpath)[pathlen] = '\0';

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) 
    apr_filepath_list_split(apr_array_header_t** pathelts,
                            const char*          liststr,
                            apr_pool_t*          p)
{
    return apr_filepath_list_split_impl(pathelts, liststr, ':', p);
}


APR_DECLARE(apr_status_t) apr_filepath_list_merge(char**              liststr,
                                                  apr_array_header_t* pathelts,
                                                  apr_pool_t*         p)
{
    return apr_filepath_list_merge_impl(liststr, pathelts, ':', p);
}


APR_DECLARE(apr_status_t) apr_filepath_encoding(int* style, apr_pool_t* p)
{
    *style = APR_FILEPATH_ENCODING_LOCALE;
    
    return APR_SUCCESS;
}

}
