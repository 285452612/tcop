#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "udb.h"

// 解析一个xml文件
xmlDocPtr getdoc(char *docname)
{
    xmlDocPtr doc;
    doc = xmlParseFile(docname);
    if (doc == NULL )
    {
        fprintf(stderr,"Document not parsed successfully. \n");
        exit(-1);
    }
    return doc;
}

// 测试节点替换
int TestReplaceNode(xmlDocPtr dest, xmlDocPtr src, const char *xpath)
{
    xmlNodePtr dest_node, src_node;

    if ((dest_node = XmlLocateNode(dest, xpath)) == NULL)
        return -1;
    if ((src_node = XmlLocateNode(src, xpath)) == NULL)
        return -1;
    // 注: 不调用xmlCopyNode, src中的src_node节点将被删除
    if ( xmlReplaceNode(dest_node, xmlCopyNode(src_node, 1)) == NULL )
        return -1;

    return 0;
}

// 给前节点增加一个新属性
int TestAddNodeProp(xmlNodePtr pNode, char *pcName, char *pcVal)
{
    xmlChar *pEncName;
    xmlChar *pEncVal;

    pEncName = ConvIn(pcName);
    if ( pEncName == NULL )
        return -1;

    pEncVal = ConvIn(pcVal);
    if ( pEncVal == NULL )
    {
        free( pEncName );
        return -1;
    }

    if ( xmlNewProp( pNode, pEncName, pEncVal ) == NULL )
    {
        free( pEncName );
        free( pEncVal );
        return -1;
    }

    free( pEncName );
    free( pEncVal );

    return 0;
}

void test1(char *docname)
{
    xmlDocPtr doc;
    xmlXPathObjectPtr result;
    xmlNodePtr extradata = NULL;
    xmlNodePtr node = NULL;
    char tmp[512];
    int i;

    // 解释一个xml文件
    doc = xmlParseFile(docname);

    // 查找节点集TrnDetail
    result = getnodeset(doc, "/UFTP/TrnDetail");
    if (result->nodesetval->nodeNr <= 0)
    {
        printf("nodeset not found\n");
        exit(-1);
    }

    for (i = 0; i < result->nodesetval->nodeNr; i++)
    {
        fprintf(stdout, "第 %d 条记录:\n", i+1);
        fprintf(stdout, "-----------------------------------------\n");

        // 移动节点到Payer
        node = XmlFetchNode(result->nodesetval->nodeTab[i], "Payer");
        // 实际应用中应该释放ConvOut后的内存
        printf("Payer = [%s]\n", XmlNodeGetContent(node));

        XmlGetNodeString(result->nodesetval->nodeTab[i], "Payer", tmp, sizeof(tmp));
        printf("Payer = [%s]\n", tmp);

        // 移动节点到ExtraData
        extradata = XmlFetchNode(result->nodesetval->nodeTab[i], "ExtraData");
        // 移动节点到ExtraData/aa
        node = XmlFetchNode(extradata, "aa");
        printf("ExtraData/aa = [%s]\n", ConvOut(xmlNodeGetContent(node)));
        XmlSetNodeString(extradata, "aa", "aaaaaaaaaa");
        XmlSetNodeString(extradata, "newnode", "新的节点内容");
        node = XmlFetchNode(extradata, "kk");
        printf("ExtraData/kk = [%s]\n", ConvOut(xmlNodeGetContent(node)));
        printf("\n");
    }

    xmlSaveFile("set.xml", doc);
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);

    // 清除libxml函数调用后占用的内存
    xmlCleanupParser();

    printf("test1 结束!\n");
    return;
}

void test2(char *buf)
{
    xmlDocPtr doc;
    xmlChar *xmlBuf;
    int len;
    char tmp[1024];

    // 解析xml格式的字符串
    printf("解析字符串:\n%s\n", buf);
    doc = xmlParseMemory(buf, strlen(buf));

    // 取 TrnCode
    printf("----------------------\n");
    XmlGetString(doc, "//MsgHdrRq/TrnCode", tmp, sizeof(tmp));
    printf("TrnCode:    [%s]\n", tmp);
    // 取 Payer
    XmlGetString(doc, "//NoteInfo/Payer", tmp, sizeof(tmp));
    printf("Payer:      [%s]\n", tmp);

    // 修改 Payer
    printf("----------------------\n");
    printf("\n设置 Payer = [信雅达]...\n");
    XmlSetString(doc, "//NoteInfo/Payer", "信雅达");

    // DUMP 到内存
    xmlDocDumpMemory(doc, &xmlBuf, &len);
    printf("----------------------\n");
    printf("DUMP到内存(%d):\n[%s]\n", len, xmlBuf);
    xmlFree(xmlBuf);

    // 保存doc到文件
    if (xmlSaveFile("out.xml", doc) != -1)
        printf("新的xml保存到 \"out.xml\".\n");

    xmlFreeDoc(doc);

    // 清除libxml函数调用后占用的内存
    xmlCleanupParser();

    printf("\ntest2 结束!\n");
    return;
}

int test3()
{
    xmlDocPtr doc1, doc2;
    xmlNodePtr node = NULL;

    // 解析xml文件
    doc1 = getdoc("dest.xml");
    doc2 = getdoc("src.xml");

    // 取根节点指针
    node = xmlDocGetRootElement(doc1);

    // 新建一个节点
    XmlNewTextChild(node, "newchild", "陈杰");

    // 替换child节点
    TestReplaceNode(doc1, doc2, "/root/child");

    node = XmlLocateNodeNew(doc2, "/root/child");
    printf("[%s]\n", xmlNodeGetContent(node));
    // 保存xml文档为文件
    xmlSaveFile("src_new.xml", doc2);
    xmlSaveFile("dest_new.xml", doc1);

    // 释放doc指针
    xmlFreeDoc(doc1);
    xmlFreeDoc(doc2);

    // 清除libxml函数调用后占用的内存
    xmlCleanupParser();

    printf("\ntest3 结束!\n");
    return 0;
}

#if 0
int main()
{
    char buf[500];
    char tmp[500];
    int len;

    printf("按回车开始执行 test1()...");
    getchar();
    test1("test1.xml");

    sprintf(buf, "<?xml version=\"1.0\" encoding=\"GB18030\"?>"
            "<UFTP><MsgHdrRq><TrnCode>0001</TrnCode></MsgHdrRq>"
            "<NoteInfo><Payer>陈杰</Payer></NoteInfo></UFTP>");
    printf("按回车开始执行 test2()...");
    getchar();
    test2(buf);

    printf("按回车开始执行 test3()...");
    getchar();
    test3(buf);

    /*
    strcpy(buf, "陈aa");
    len = Iconv(buf, strlen(buf), "GB18030", tmp, sizeof(tmp), "UTF-8");

    printf("[%d][%s]\n", len, buf);
    printf("[%d][%s]\n", len, tmp);
    */
    return 0;
}
#endif

int notetype_c2b(char *notetype, char *bktype, char *bkcode)
{
    xmlDocPtr doc;
    xmlNodePtr node;
    char path[256];
    char *p;

    *bktype = *bkcode = 0x00;
    sprintf(path, "%s/etc/map.xml", getenv("APP_DIR"));
    if ((doc = xmlParseFile(path)) == NULL)
        return -1;

    sprintf(path, "//notetype_c2b/mapping[@name='%s']", notetype);
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    if ((p = XmlNodeGetAttrText(node, "value")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bktype, p);
    free(p); p = NULL;

    if ((p = XmlNodeGetAttrText(node, "bkcode")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bkcode, p);

    free(p);
    xmlFreeDoc(doc);
    return 0;
}
int main()
{
    xmlDocPtr doc, doc1, doc2;
    xmlNodePtr node = NULL, cur, scur;
    char *grid;
    char path[1024], buf1[100], buf2[100];
    int rc;

    // 解析xml文件
    doc = getdoc("../etc/cvt.xml");

    sprintf(path, "/Root/Transaction[@code='sz37']/Response");
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        err_log("查找节点[%s]失败!", path);
        xmlFreeDoc(doc);
        return -1;
    }

    for( cur = node->children; cur != NULL; cur = cur->next )
    {
        if (cur->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp(cur->name, "grid"))
        {
            for (scur = cur->children; scur != NULL; scur = scur->next)
            {
                if (scur->type != XML_ELEMENT_NODE)
                    continue;
                printf("anode=[%s]\n", scur->name);
            }
        }
        else
            printf("node=[%s]\n", cur->name);
    }

    // 释放doc指针
    xmlFreeDoc(doc);

    rc = notetype_c2b("09", buf1, buf2);
    printf("[%d][%s][%s]\n", rc, buf1, buf2);
    // 清除libxml函数调用后占用的内存
    xmlCleanupParser();

    return 0;
}
