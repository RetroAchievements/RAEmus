using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RAInterface
{
    public enum WebRequest
    {
        //	Login
        RequestLogin,

        //	Fetch
        RequestScore,
        RequestNews,
        RequestPatch,
        RequestLatestClientPage,
        RequestRichPresence,
        RequestAchievementInfo,
        RequestLeaderboardInfo,
        RequestCodeNotes,
        RequestFriendList,
        RequestUserPic,
        RequestBadgeIter,

        //	Submit
        RequestPing,
        RequestPostActivity,
        RequestUploadBadgeImage,
        RequestSubmitAwardAchievement,
        RequestSubmitCodeNote,
        RequestSubmitLeaderboardEntry,
        RequestSubmitAchievementData,
        RequestSubmitTicket,

        //etc
    }

    public class RAHttpRequest
    {
        private static string[] RequestArg =
        {
	        "login",

	        "score",
	        "news",
	        "patch",
	        "",						//	TBD RequestLatestClientPage
	        "richpresencepatch",
	        "achievementwondata",
	        "lbinfo",
	        "codenotes2",
	        "getfriendlist",
	        "",						//	TBD RequestUserPic
	        "badgeiter",

	        "",						//	TBD RequestPing (ping.php)
	        "postactivity",
	        "",						//	TBD RequestBadgeImage
	        "awardachievement",
	        "submitcodenote",
	        "",						//	TBD: Complex!!! See requestsubmitlbentry.php
	        "uploadachievement",
            "submitticket",

            //"requestlogin.php",
            //"requestpostactivity.php",
            //"requestfriendlist.php",
            //"requestachievement.php",
            //"requestachievementinfo.php",
            //"requestaddfriend.php",
            //"requestallgametitles.php",
            //"requestallmyprogress.php",
            //"requestassociatefb.php",
            //"requestbadgenames.php",
            //"requestchangeemailaddress.php",
            //"requestchangefb.php",
            //"requestchangefriend.php",
            //"requestchangepassword.php",
            //"requestchangesiteprefs.php",
            //"requestcodenotes.php",
            //"requestcreatenewlb.php",
            //"requestcreateuser.php",
            //"requestcurrentlyactiveplayers.php",
            //"requestcurrentlyonlinelist.php",
            //"requestdeletelb.php",
            //"requestdeletemessage.php",
            //"requestfetchmessage.php",
            //"requestfriendlist.php",
            //"requestgameid.php",
            //"requestgametitles.php",
            //"requesthashlibrary.php",
            //"requestlbinfo.php",
            //"requestlogin.php",
            //"requestmergegameids.php",
            //"requestmessageids.php",
            //"requestmodifygame.php",
            //"requestmodifynews.php",
            //"requestmodifytopic.php",
            //"requestnewpic.php",
            //"requestnews.php",
            //"requestpatch.php",
            //"requestpostactivity.php",
            //"requestpostcomment.php",
            //"requestreconstructsiteawards.php",
            //"requestremovefb.php",
            //"requestresendactivationemail.php",
            //"requestresetachievements.php",
            //"requestresetlb.php",
            //"requestrichpresence.php",
            //"requestscore.php",
            //"requestscorerecalculation.php",
            //"requestsearch.php",
            //"requestsendmessage.php",
            //"requestsetmessageread.php",
            //"requestsubmitalt.php",
            //"requestsubmitcodenote.php",
            //"requestsubmiteditpost.php",
            //"requestsubmitforumtopic.php",
            //"requestsubmitgametitle.php",
            //"requestsubmitlbentry.php",
            //"requestsubmitticket.php",
            //"requestsubmittopiccomment.php",
            //"requestsubmitusermotto.php",
            //"requestsubmitvid.php",
            //"requestunlocks.php",
            //"requestunlockssite.php",
            //"requestupdateachievement.php",
            //"requestupdatelb.php",
            //"requestupdateticket.php",
            //"requestupdateuser.php",
            //"requestuploadachievement.php",
            //"requestuploadbadge.php",
            //"requestuserplayedgames.php",
            //"requestvote.php",
        };


        const string BaseURL = "http://retroachievements.org/dorequest.php";
        public string TargetURL { get; private set; }
        public FormUrlEncodedContent Args { get; private set; }
        public WebRequest RequestType { get; private set; }

        public class PostArgs : List<KeyValuePair<string, string>>
        {
        }

        public RAHttpRequest(WebRequest type, PostArgs args)
        {
            TargetURL = BaseURL;
            RequestType = type;
            args.Add(new KeyValuePair<string, string>("r", RequestArg[(int)type]));
            Args = new FormUrlEncodedContent(args);
        }
    }

}