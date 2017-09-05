/*
 * 功能: 读取目录下所有子目录的大小等信息
*/

#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <sys/types.h> 
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>

using namespace std;

string g_strdir;
list<string> g_listSubDirs; 

string getExtensionName(string s)
{
    std::size_t found = s.rfind('.');
    if(found==std::string::npos)
    {
        return string("");
    }
    else
    {
        return s.substr(found+1);
    }
}

bool genSubDirs(string path) 
{
    DIR *pDir ;
    struct dirent *ent;
    string subpath;
    pDir=opendir(path.c_str());
    if(pDir==NULL)
    {
        return 0;
    }
    while((ent=readdir(pDir))!=NULL)
    {
        if(ent->d_type & DT_DIR)
        {  
            string dirname = ent->d_name;
            if(dirname=="." || dirname=="..")  
                continue;
            subpath=path+"/"+dirname;
            printf("find subpath:%s\n",subpath.c_str());
            g_listSubDirs.push_back(subpath);
        }
        else
        {
            continue;
        }
    }
    return 1;
}

class DirInfo
{
public:
    unsigned long long m_maxfilesize;
    unsigned long long m_minfilesize;
    unsigned long long m_filenum;
    unsigned long long m_totalsize;
    struct info
    {
        unsigned long long m_maxfilesize;
        unsigned long long m_minfilesize;
        unsigned long long m_filenum;
        unsigned long long m_totalsize;
        info(){m_maxfilesize=m_minfilesize=m_filenum=m_totalsize=0;}
    };
    
    string humanReadable(unsigned long long n)
    {
        //Too Simple Maybe
        string str="";
        char s[64];
        if(n<1024)
        {
            sprintf(s,"%d",(int)n);
            str=s;
            str+="B";
        }
        else if(n<1024*1024)
        {
            sprintf(s,"%.1f",n/1024.0);
            str=s;
            str+="KB";
        }
        else if(n<1024UL*1024*1024)
        {
            sprintf(s,"%.1f",n/(1024.0*1024.0));
            str=s;
            str+="MB";
        }
        else if(n<1024UL*1024*1024*1024)
        {
            sprintf(s,"%.1f",n/(1024.0*1024.0*1024.0));
            str=s;
            str+="GB";
        }
        else
        {
            //not occur for me!
        }
        return str;
    }
    map<string, info> m_statistics;
    DirInfo(string path)
    {
        m_maxfilesize=0;
        m_minfilesize=0;
        m_filenum=0;
        m_totalsize=0;
        trdir(path);
    }
    void trdir(string path)
    {
        DIR *pDir ;
        struct dirent *ent;
        string subpath;
        pDir=opendir(path.c_str());
        if(pDir==NULL)
        {
            return;
        }
        while((ent=readdir(pDir))!=NULL)
        {
            if(ent->d_type & DT_DIR)
            {  
                string dirname = ent->d_name;
                if(dirname=="." || dirname=="..")  
                    continue;
                trdir(path+"/"+dirname);
            }
            else if(ent->d_type & DT_REG)
            {
                struct stat filestat;
                string name=ent->d_name;
                string fullpath=path+"/"+name;
                stat(fullpath.c_str(), &filestat);
                unsigned long long filesize = filestat.st_size;

                if(m_filenum==0)
                {
                    m_minfilesize=m_maxfilesize=filesize;
                }
                else
                {
                    if(filesize>m_maxfilesize)
                    {
                        m_maxfilesize=filesize;
                    }
                    if(filesize<m_minfilesize)
                    {
                        m_minfilesize=filesize;
                    }
                }
                m_totalsize+=filesize;
                m_filenum++;
                
                string extname=getExtensionName(ent->d_name);
                if(m_statistics[extname].m_filenum==0)
                {
                    m_statistics[extname].m_minfilesize=m_statistics[extname].m_maxfilesize=filesize;
                }
                else
                {
                    if(filesize>m_statistics[extname].m_maxfilesize)
                    {
                        m_statistics[extname].m_maxfilesize=filesize;
                    }
                    if(filesize<m_statistics[extname].m_minfilesize)
                    {
                        m_statistics[extname].m_minfilesize=filesize;
                    }
                }
                m_statistics[extname].m_totalsize+=filesize;
                m_statistics[extname].m_filenum++;
                
                continue;
            }
        }
        closedir(pDir); 
    }
    ~DirInfo(){};
};

int main(int argc, char * argv[])
{
    if(argc==2)
    {
        g_strdir=argv[1];
    }
    else if(argc==1)
    {
        g_strdir=".";
    }
    else
    {
        return 0;
    }
    
    printf("Start For Directory: %s\n",g_strdir.c_str());
    if(genSubDirs(g_strdir.c_str())==0)
    {
         printf("Wrong Directory.\n");
         return 0;
    }
    if(g_listSubDirs.size()==0)
    {
        printf("Directory Contains No Subdirs\n");
        return 0;
    }
    
    FILE *file;
    file = fopen("./out.txt", "wt+");
    if(file == NULL)
    {
        printf("Failed To Create Output File.\n");
        return 0;
    }
    
    list<string>::iterator i;

    for (i = g_listSubDirs.begin(); i != g_listSubDirs.end(); ++i)
    {        
        printf("%s Details:\n",i->c_str());
        fprintf(file, "%s Details:\n",i->c_str());
        
        DirInfo info(*i);
        
        printf("ALL:         | ");
        printf("num=%6lld | ",info.m_filenum);
        printf("min=%8s | ",info.humanReadable(info.m_minfilesize).c_str());
        printf("max=%8s | ",info.humanReadable(info.m_maxfilesize).c_str());
        printf("total=%8s | ",info.humanReadable(info.m_totalsize).c_str());
        printf("\r\n");
        fprintf(file, "ALL:         | ");
        fprintf(file, "num=%6lld | ",info.m_filenum);
        fprintf(file, "min=%8s | ",info.humanReadable(info.m_minfilesize).c_str());
        fprintf(file, "max=%8s | ",info.humanReadable(info.m_maxfilesize).c_str());
        fprintf(file, "total=%8s | ",info.humanReadable(info.m_totalsize).c_str());
        fprintf(file, "\r\n");
        
        //sort by something???
        for (map<string, DirInfo::info>::iterator iter = info.m_statistics.begin(); 
             iter != info.m_statistics.end();  
             ++iter) 
        {
            printf("[%10s] | ",iter->first.c_str());//10 is enough for me!!
            printf("num=%6lld | ",iter->second.m_filenum);//999999 is enough for me!!
            printf("min=%8s | ",info.humanReadable(iter->second.m_minfilesize).c_str());
            printf("max=%8s | ",info.humanReadable(iter->second.m_maxfilesize).c_str());
            printf("total=%8s | ",info.humanReadable(iter->second.m_totalsize).c_str());
            printf("\r\n");
            fprintf(file, "[%10s] | ",iter->first.c_str());
            fprintf(file, "num=%6lld | ",iter->second.m_filenum);
            fprintf(file, "min=%8s | ",info.humanReadable(iter->second.m_minfilesize).c_str());
            fprintf(file, "max=%8s | ",info.humanReadable(iter->second.m_maxfilesize).c_str());
            fprintf(file, "total=%8s | ",info.humanReadable(iter->second.m_totalsize).c_str());
            fprintf(file, "\r\n");
        }
        
        printf("\r\n");
        printf("\r\n");
        fprintf(file, "\r\n");
        fprintf(file, "\r\n");
    }
    
    fclose(file);
    printf("Finished. Press Any Key To Exit\n");
    getchar();
    return 0;
}

