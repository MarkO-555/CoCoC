#include "cp.h"

expand(ln,loop,lpcntr)   /* Macro expansion rtn */
char *ln;    /* ln[] must start and end with a space character */
int *loop,*lpcntr;
{
int c,i,x,os,m,n,lp[32],lpc,lpflag;
int d,e,f,j,k,l,tcnt;
char tptr[80],*dptr,*lnptr,*lnptr2,*lptr;
char macargs[MAX_ARGS][MAX_LENGTH];
char exmacargs[MAX_ARGS][MAX_LENGTH];
char buf[LINEMAX+3], dtok[LINEMAX+3];
char *idptr[80];
int mname,mname2,idcntr;

/* SETUP LOOP AND LPCNTR */

    if (!*lpcntr)
    {
        lpc=0;
        lpcntr=&lpc;
        loop=lp;
    }

/* PRESCAN LN TO MARK IDENTIFIER FOR MACRO SEARCHES */

    idcntr=0;
    lptr=ln;
    while(lptr=strchr(lptr,' '))
    {
        ++lptr;
        if (IDNT_INIT(*lptr))
        {
            if (*lptr!='L' || (*(lptr+1)!='\"' && *(lptr+1)!='\''))
                idptr[idcntr++]=lptr;
        }
    }

/* SEARCH LN FOR MACRO CALLS */

    for (x=0;x<idcntr;++x)  /* loop for each identifier */
    {
    for (i=0;i<defcntr;++i) /* loop for each definition name */
    {
        if (strcmp2(idptr[x],defnam[i]))  /* returns 1 if match else 0 */
        {
            lpflag=FALSE;
            for (n=0;n<*lpcntr;++n)
            {
                if (loop[n]==i)
                {
                    lpflag=TRUE;
                    break;
                }
            }
            if (lpflag)
                continue;
            loop[(*lpcntr)++]=i;

            lptr=idptr[x];
            c=lptr-ln;
            *buf=0;
            if (defarg[i])   /* If a macro */
            {
                e=0;
                f=c+strlen(defnam[i]);  /* skip to ( */
                if (ln[f]!='(')
                {
                    doerr(13,f);    /* macro syntax error */
                    return;
                }
                f+=2;   /* point to 1st arg */
                do      /* loop for each arg */
                {
                    l=toksrch(ln,",",0,99,99,f,&tcnt);
                    if (l==ERROR)
                    {   /* either get , separator or ) arg terminator */
                        l=toksrch(ln,")",-1,99,99,f,&tcnt);
                        if (l==ERROR)
                        {
                            doerr(13,f);  /* macro syntax error */
                            return;
                        }
                    }
                    if (l==f)   /* not sure of result when l pts to ) */
                    {
                        doerr(13,f); /* macro syntax error */
                        return;
                    }
                    strncpy(macargs[e],&ln[f],l-f-1);
                    macargs[e][l-f-1]=0;    /* put arg in array */
                    strcpy(exmacargs[e],macargs[e]); /* put in expand array */
                    ++e;
                    f=l+2;
                } while (ln[l]==',');
                for (j=0;j<e;++j)   /* loop to expand each arg */
                {
                    expln(exmacargs[j],NULL,NULL);
                }
                if (tstargs(i)!=e)
                {
                    doerr(14,d);    /* wrong # of args */
                    return;
                }
                k=1;

        /* Search token sequence for args */

                while (getoknum(deftok[i],tptr,k++))
                {
                    if (toksrch(defarg[i],tptr,99,99,99,0,&tcnt)!=ERROR)
                    {       /* token found is arg */
                        if (!strcmp(getoknum(deftok[i],tptr,k-2),"#"))
                        {
                            addqmac(macargs[tcnt-1]);
                            strcat(buf,macargs[tcnt-1]);
                            strcat(buf," ");
                        }
                        else if (!strcmp(getoknum(deftok[i],tptr,k-2),"##") || !strcmp(getoknum(deftok[i],tptr,k),"##"))
                        {
                            strcat(buf,macargs[tcnt-1]);
                            strcat(buf," ");
                        }
                        else
                        {
                            strcat(buf,exmacargs[tcnt-1]);
                            strcat(buf," ");    /* use expanded args */
                        }
                    }
                    else    /* token is not argument */
                    {
                        if (strlen(tptr)==1 && *tptr=='#')
                            ;
                        else if (strlen(tptr)==2 && *tptr=='#' && *(tptr+1)=='#')
                            buf[strlen(buf)-1]=0;
                        else
                        {
                            strcat(buf,tptr);   /* substitute in token */
                            strcat(buf," ");
                        }
                    }
                }
                /* substitute entire macro into ln[] */
                buf[strlen(buf)-1]=0;   /* drop final space */
                mname=l-c+1;    /* get length of macro */
                strcpy(dtok,&ln[l+1]);  /* save end of line */
                lnptr=&ln[c];

                strcpy(lnptr,buf);       /* put expanded tok-seq in ln[] */

                expln(lnptr,loop,lpcntr);          /* expand tok-seq */
                mname2=strlen(lnptr);

                strcat(lnptr,dtok);   /* replace end of ln */

    os=0;
    for (m=x+1;m<idcntr;++m)    /* adjust idptr */
    {
        if (idptr[m]<&ln[l])
            ++os;
        else
            idptr[m-os]=idptr[m]+mname2-mname;
    }
    idcntr-=os;
            }
            else    /* Not a macro */
            {
                mname=strlen(defnam[i])-1;  /* get length of identifier */
                strcpy(buf,lptr+mname);     /* save end of line */
                switch(i)
                {
                    case 0: /* __LINE__ */
                        itoa(_line_-1,lptr);
                    break;
                    case 1: /* __FILE__ */
                        strcpy(lptr,ifnbuf[fptr]);
                    break;
                    default:
                        strcpy(lptr,deftok[i]);   /* copy tok-seq into line */
                        expln(lptr,loop,lpcntr);  /* expand token sequence */
                    break;
                }
                mname2=strlen(lptr);
                strcat(ln,buf);   /* replace end of line */
                for (l=x+1;l<idcntr;++l)   /* adjust idptr */
                    idptr[l]+=mname2-mname;
            }
            if (*lpcntr)
                --*lpcntr;
            break;
        }
    }
    }
}

expln(ln,l,lc)   /* Inserts beginning and ending space then calls expand */
char *ln;
int *l,*lc;
{
char buf[LINEMAX+3];

    *buf=' ';
    strcpy(&buf[1],ln);
    strcat(buf," ");
    expand(buf,l,lc);
    buf[strlen(buf)-1]=0;   /* Kill final space in line */
    strcpy(ln,&buf[1]);
}

#asm
strcmp2:
 pshs u
 ldu 4,s   (s1)
 ldx 6,s   (s2)
STRLP0
 ldb ,x+
 beq STRMTCH
 cmpb ,u+
 beq STRLP0
 clra
 clrb
 puls u,pc
STRMTCH
 ldd #1
 puls u,pc
#endasm
