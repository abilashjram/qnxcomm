#include <gtest/gtest.h>
#include <thread>

#include "qnxcomm.h"


namespace {

void receiverthread(int chid)
{
   char buf[80];
   memset(buf, 0xFF, sizeof(buf));
   
   struct _msg_info info;
      
   int rcvid = MsgReceive(chid, buf, sizeof(buf), &info);
   EXPECT_GT(rcvid, 0);
   EXPECT_EQ(0, strcmp(buf, "Hallo Welt"));
   
   int rc = MsgReply(rcvid, 0, buf, strlen(buf) + 1);
   int error = errno;
   EXPECT_EQ(-1, rc);
   EXPECT_EQ(ESRCH, error);
   
   EXPECT_EQ(0, info.nd);
   EXPECT_EQ(::getpid(), info.pid);   
   EXPECT_EQ(11, info.msglen);
   EXPECT_EQ(11, info.srcmsglen);
   EXPECT_EQ(0, info.dstmsglen);   

   EXPECT_NE(0, info.flags & QNX_FLAG_NOREPLY);   
   
   // -------------------------------------------------------------
   
   rcvid = MsgReceive(chid, buf, sizeof(buf), 0);
   EXPECT_GT(rcvid, 0);
   EXPECT_EQ(0, strcmp(buf, "Hallo Welt"));   
   
   rc = MsgError(rcvid, EINVAL);
   error = errno;
   EXPECT_EQ(-1, rc);
   EXPECT_EQ(ESRCH, error);
}

}


TEST(MsgSendNoReply, basics) 
{
   int chid = ChannelCreate(0);
   EXPECT_GT(chid, 0);
   
   int coid = ConnectAttach(0, 0, chid, 0, 0);
   EXPECT_GT(coid, 0);
 
   std::thread t(&receiverthread, chid);
   
   // now send message and wait for reply      
   char buf[80];   
   strcpy(buf, "Hallo Welt");
   
   int rc = MsgSendNoReply(coid, buf, strlen(buf) + 1);
   EXPECT_EQ(0, rc);
   
   rc = MsgSendNoReply(coid, buf, strlen(buf) + 1);
   EXPECT_EQ(0, rc);
   
   strcpy(buf, "Hallo Welt");
   rc = MsgSendNoReply(4711, buf, strlen(buf) + 1);
   EXPECT_EQ(-1, rc);
   EXPECT_EQ(EBADF, errno);
   
   t.join(); 
   
   EXPECT_EQ(0, ChannelDestroy(chid));
   EXPECT_EQ(0, ConnectDetach(coid));  
}

