#include<stdio.h>
#include<string.h>
int count;
char dict[3000][2][64];
char *get_desc(char *name)
{
    int i;
    for(i=0;i<count;i++)
        if(memcmp(name,dict[i][0],6)==0)
            return dict[i][1];
    return "";
}
int main(int argc,char *argv[])
{
    int i;
    char temp[256],*name,*desc;
    FILE *pdict=fopen("/home/tcop/bin/dict","r");
    FILE *iput=fopen(argv[1],"r");
    FILE *oput=fopen(argv[2],"w");
    for(count=0;fscanf(pdict,"%s%s",dict[count][0],dict[count][1])!=EOF;count++);
    fclose(pdict);
    if(argc==2)
    {
        printf("[%s]:[%s]\n",argv[1],get_desc(argv[1]));
        return 0;
    }
    if(argc<3)
    {
        printf("Usage: %s inputfile outputfile\n",argv[0]);
        return 0;
    }
    while(fgets(temp,256,iput)!=NULL)
    {
        if(temp[1]!='I')//is not Item
            fprintf(oput,"%s",temp);
        else
        {
            name=strstr(temp,"name=")+6;
            desc=strstr(temp,"desc=");
            *desc=0;
            if(*name=='\"'||*name=='/')//name is null or start with '/'
            {
                name=strstr(temp,"value=")+7;
                fprintf(oput,"%sdesc=\"%s\"/>\n",temp,get_desc(name));
            }
            else
            {
                fprintf(oput,"%sdesc=\"%s\"/>\n",temp,get_desc(name));
            }
        }
    }
    fclose(iput);
    fclose(oput);
    return 0;
}
