#ifndef REGION_COMMON_H_
#define REGION_COMMON_H_

#include "pub.h"
#include "dbutil.h"
#include "pack.h"

#define xmlGetVal(doc, path, val) XmlGetString(doc, path, val, sizeof(val))
#define XMLGetVal(doc, path, val) XmlGetString(doc, path, val, sizeof(val))

/*
 * ���ݽ��ױ��Ľ��м�Ѻ
 *
 * tctcode: ͬ�ǽ�����
 * doc: ͬ�ǽ��ױ���
 *
 * ���� �ɹ� 0 ʧ�� E_SYS_ADDDIGEST������
 */
extern int AddDigest(xmlDoc *doc, int tctcode);

/*
 * ���ݽ��ױ��Ľ��к�Ѻ
 *
 * tctcode: ͬ�ǽ�����
 * doc: ͬ�ǽ��ױ���
 *
 * ���� �ɹ� 0 ʧ�� E_SYS_CHKDIGEST������
 */
extern int CheckDigest(xmlDoc *doc, int tctcode);

#endif
