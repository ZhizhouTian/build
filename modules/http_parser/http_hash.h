#ifndef __HTTP_HASH_H_
#define __HTTP_HASH_H_

enum {
	HTTP_Url = 0,
	HTTP_Host ,
	HTTP_User_Agent ,
	HTTP_Referer ,
	HTTP_Seq,
	HTTP_Pragma , // 5
	HTTP_Accept ,
	HTTP_Cookie ,
	HTTP_Keep_Alive ,
	HTTP_Cache_Control ,
	HTTP_Content_Encoding , // 10
	HTTP_Content_Language ,
	HTTP_Content_Type ,
	HTTP_Range ,
	HTTP_Content_Length ,
	HTTP_Connection , // 15
	HTTP_Date ,
	HTTP_Transfer_Encoding ,
	HTTP_Upgrade ,
	HTTP_Via ,
	HTTP_Accept_Charset , // 20
	HTTP_Accept_Encoding ,
	HTTP_Accept_Language ,
	HTTP_Authorization ,
	HTTP_From ,
	HTTP_If_Modified_Since , // 25
	HTTP_If_Match ,
	HTTP_If_None_Match ,
	HTTP_If_Range ,
	HTTP_If_Unmodified_Since ,
	HTTP_Max_Forwards , // 30
	HTTP_Proxy_Authorization ,
	HTTP_Age ,
	HTTP_Location ,
	HTTP_Proxy_Authenticate ,
	HTTP_Public , // 35
	HTTP_Retry_After ,
	HTTP_Server ,
	HTTP_Vary ,
	HTTP_Warning ,
	HTTP_WWW_Authenticate , // 40
	HTTP_Allow ,
	HTTP_Content_Base ,
	HTTP_Content_Location ,
	HTTP_Content_MD5 ,
	HTTP_Content_Range , // 45
	HTTP_Etag ,
	HTTP_Expires ,
	HTTP_Last_Modified ,
	HTTP_Version ,
	HTTP_Status , // 50
	HTTP_Priv,
	HTTP_Ext,
	HTTP_Body,	
	HTTP_Priv2,
	HTTP_Priv3, //55

	HTTP_X_Request_With,
	HTTP_Set_Cookie, 
	
	HTTP_MAX, // 58 
};
#endif
