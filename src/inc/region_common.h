#ifndef REGION_COMMON_H_
#define REGION_COMMON_H_

#include "pub.h"
#include "dbutil.h"
#include "pack.h"

#define xmlGetVal(doc, path, val) XmlGetString(doc, path, val, sizeof(val))
#define XMLGetVal(doc, path, val) XmlGetString(doc, path, val, sizeof(val))

/*
 * 根据交易报文进行加押
 *
 * tctcode: 同城交易码
 * doc: 同城交易报文
 *
 * 返回 成功 0 失败 E_SYS_ADDDIGEST或其它
 */
extern int AddDigest(xmlDoc *doc, int tctcode);

/*
 * 根据交易报文进行核押
 *
 * tctcode: 同城交易码
 * doc: 同城交易报文
 *
 * 返回 成功 0 失败 E_SYS_CHKDIGEST或其它
 */
extern int CheckDigest(xmlDoc *doc, int tctcode);

#endif
