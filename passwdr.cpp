/*
 * passwdr
 *
 * Copyright (C) 2013 geekprojects.com	
 * 
 * Includes base64 code that is: 
 *  - Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termio.h>
#include <fcntl.h>

#include <string>

#include "sha1.h"

using namespace std;

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char hashword[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";

static void encodeblock( unsigned char *in, unsigned char *out, int len )
{
    out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
    out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

string getPassword(string prompt)
{
    string password = "";
    struct termio tsave, chgit;

    printf("%s: ", prompt.c_str());
    fflush(stdout);

    // Save the current state
    if (ioctl(0, TCGETA, &tsave) == -1)
    {
        printf("Failed to store terminal settings!\n");
        exit(1);
    }
    chgit = tsave;

    // Turn off canonial mode and echoing
    chgit.c_lflag &= ~(ICANON|ECHO);
    chgit.c_cc[VMIN] = 1;
    chgit.c_cc[VTIME] = 0;
    if (ioctl(0, TCSETA, &chgit) == -1)
    {
        printf("Failed to modify terminal settings!\n");
        exit(1);
    }

    while (1)
    {
        int c = getchar();
        /* CR is ascii value 13, interrupt is -1, control-c is 3 */
        if (c == '\r' || c == '\n' || c == '\b' || c == -1 || c == 3)
        {
            break;
        }

        if (isprint(c))
        {
            password += c;
        }
    }

    printf("\n");

    if (ioctl(0, TCSETA, &tsave) == -1)
    {
        printf("Failed to restore terminal settings!\n");
        exit(1);
    }
    return password;
}

int main(int argc, char** argv)
{
    char* domain;

    if (argc < 2)
    {
        return 0;
    }

    domain = argv[1];

    string password = getPassword("Password");
    string password2 = getPassword("Retype password");

    if (password != password2)
    {
        printf("Passwords do not match\n");
        return 0;
    }

    //string str = string(domain) + password + "passwdr";
    string str = password + string(domain) + "passwdr" + string(domain);

    SHA1Context sha;
    uint8_t digest[20];
    int err;

    err = SHA1Reset(&sha);
    if (err)
    {
        return 0;
    }

    err = SHA1Input(&sha, (uint8_t*)str.c_str(), str.length());
    if (err)
    {
        return 0;
    }

    err = SHA1Result(&sha, digest);
    if (err)
    {
        return 0;
    }

    int i;
    string passwd = "";
    if (false)
    {
        for (i = 0; i < 18; i += 3)
        {
            unsigned char out[4];
            encodeblock(digest + i, out, 3);
            passwd += out[0];
            passwd += out[1];
            passwd += out[2];
            passwd += out[3];
        }
    }
    else
    {
        //hashword
        int hashcharlen = strlen(hashword);
        for (i = 0; i < 16; i++)
        {
            int b = ((char)digest[i]);
            b += 128;
            passwd += hashword[b % hashcharlen];
        }
    }
    printf("passwdr: %s\n", passwd.c_str());

    return 0;
}

