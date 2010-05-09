using System;
using System.Text.RegularExpressions;
using System.IO;
using System.Collections;
namespace IPLFGen
{
	/// <summary>
	/// Main IPLFGen class
	/// </summary>
	class Main_IPLFGen
	{
		static Regex versionRe = new Regex(@"
												\n[^\n]*\ VERSIONINFO.*?
												PRODUCTVERSION\s*?(?<Major>\d+)\s*,\s*(?<Minor>\d+)\s*,\s*(?<Revision>\d+)\s*,\s*(?<Build>\d+)
											",RegexOptions.Singleline | RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace);
		static Regex ctrlsRe = new Regex(@"
												\n(?<DialogID>[0-9]*?)\ DIALOGEX[^\n]*
												\nSTYLE .*?
												\nCAPTION\s*?\""(?<DialogCaption>[^\n]*?)\"".*?
												\nFONT .*?
												\nBEGIN[\n\r\s]*?
												(?:
													(?:\n\s*\w*\s*\""(?<ControlCaption>[^\n]*?)\""[\n\r\s]*?,[\n\r\s]*?(?<ControlID>\d*?),[^\n]*?)
													|
													(?:\n[^\n]*?)
												)+
												[\n\r\s]*?END[\n\r\s]*?
										",RegexOptions.Singleline | RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace);
		static Regex strsRe = new Regex(@"
											\nSTRINGTABLE.*?
											\nBEGIN.*?
											(?:\n\s*(?<StringID>[0-9\(\)\ \+]*?)[\s\r\n]*?\""(?<String>(?:.|\""\"")*?)\""[\r\n]*?)+
											END
										",RegexOptions.Singleline | RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace);
		static Regex skipStrsRe = new Regex(@"\n\#pragma\ comment\(\""user\""\,IPLFGen_skip_string\:(?<StringID>[^\n]*?)\)"
			,RegexOptions.Singleline | RegexOptions.Compiled | RegexOptions.IgnorePatternWhitespace | RegexOptions.IgnoreCase);
		/// <summary>
		/// Il punto di ingresso principale dell'applicazione.
		/// </summary>
		[STAThread]
		static int Main(string[] args)
		{
			ArrayList skipStrings=new ArrayList();
			if(args.Length != 0)
			{
				Console.WriteLine(GetAssemblyAbout(null));
				Console.WriteLine("Generates an IrfanPaint language file from the IrfanPaint.rc resource file.");
				Console.WriteLine("The input is read from stdin.");
				Console.WriteLine("To obtain a valid language file the input file must be preprocessed.");
				Console.WriteLine();
				Console.WriteLine("Usage example:");
				Console.WriteLine("  cl /D \"RC_INVOKED\" /E IrfanPaint.rc | IPLFGen > IP_English.lng");
				return 0;
			}
			string inString="",tstr;
			MatchCollection matches;
			info.lundin.Math.ExpressionParser parser = new info.lundin.Math.ExpressionParser();
			while((tstr=Console.ReadLine())!=null)
				inString+=tstr+"\n";
			//Comments
			Console.WriteLine(";Language file template generated on " + DateTime.Now.ToString("r") + " by " + GetAssemblyAbout(null));
			Console.WriteLine(@";=== FILE STRUCTURE ===");
			Console.WriteLine(@";The language file contains a [FileInfo] section, a [General] section and some numbered sections (e.g. [101], [102], ...)");
			Console.WriteLine(@";The [FileInfo] sections contains informations about the translator, the language name and the target IrfanPaint version.");
			Console.WriteLine(@";In the TargetVersion value you can insert a * so that the comparison between the TargetVersion and the IrfanPaint version stop there.");
			Console.WriteLine(@";For example, if you want to make the language file work with all the 0.4.11 builds, just insert 0.4.11.*; be careful, always insert the asterisk after a dot.");
			Console.WriteLine(@";The [General] section contains the strings of the string table; they are usually strings that are displayed in error messages, tooltips, ...");
			Console.WriteLine(@";Each numbered section contains the strings of a dialog; the strings numbers are the IDs of the controls in which they will be displayed.");
			Console.WriteLine(@";The 0 string is the title of the dialog.");
			Console.WriteLine(@";=== ACCELERATORS ===");
			Console.WriteLine(@";The & symbol is put before the accelerator letter; for example if you find a &Bold it means that the button that contains that text can be pressed also pressing ALT+B.");
			Console.WriteLine(@";You can change the accelerator letter (e.g. in Italian &Italic becomes &Corsivo), but be careful: the accelerators of the same dialog must not conflict.");
			Console.WriteLine(@";=== ESCAPE SEQUENCES ===");
			Console.WriteLine(@";The recognised escape sequences are:");
			Console.WriteLine(@";   \n      newline          (ASCII 0x0A)");
			Console.WriteLine(@";   \r      carriage return  (ASCII 0x0D)");
			Console.WriteLine(@";   \\      backslash        (ASCII 0x5C)");
			Console.WriteLine(@";Unrecognized escape sequences are treated as left as they are.");
			Console.WriteLine(@";In most of controls to make a new line you need to insert a \r\n.");
			Console.WriteLine(@";=== PLACEHOLDERS ===");
			Console.WriteLine(@";Text enclosed between two ""%"" is a placeholder for something else that will be put there at runtime.");
			Console.WriteLine(@";You can move the placeholder where you want in the string, so you'll be able to arrange the sentence as you like more.");
			Console.WriteLine(@";These text placeholders are specific of each string; you cannot use the placeholders of a string in another.");
			Console.WriteLine(@";The placeholders names are usually self-descriptive. Anyway, here's a list of the currently used placeholders:");
			Console.WriteLine(@";   %translator%    Translator name as stated in the [FileInfo]\TranslatorName key.");
			Console.WriteLine(@";   %language%      Language name as stated in the [FileInfo]\LanguageName key.");
			Console.WriteLine(@";=== COMMENTS ===");
			Console.WriteLine(@";If a row begins with a ; it is a comment; it is completely ignored by the parser.");
			Console.WriteLine(@";You can put any comment you want, but all the comments will be stripped out when the file will be distributed to reduce file size unless you ask to keep them.");
			Console.WriteLine(@";It is very appreciated if you strip out all the comments (including these that you are reading now) by yourself. Thank you!");
			Console.WriteLine(@";=== MISC ===");
			Console.WriteLine(@";If you do not translate a string it will remain untranslated in the interface, no error message (like ""incomplete language file"") will be issued.");
			//FileInfo section
			string version;
			Console.WriteLine("[FileInfo]");
			Console.WriteLine("TranslatorName=Put your name here");
			Console.WriteLine("LanguageName=Put the language name (in its language, not in English) here");
			//Get the version
			{
				Match mc = versionRe.Match(inString);
				try
				{
					version = mc.Groups["Major"].Value + "." + mc.Groups["Minor"].Value + "." + mc.Groups["Revision"].Value + ".*" /*+ mc.Groups["Build"].Value*/;
				}
				catch
				{
					version=null;
				}
				if(version!=null && version != "...")
				{
					Console.Write("TargetVersion=");
					Console.WriteLine(version);
				}
			}
			//String skip
			matches=skipStrsRe.Matches(inString);
			foreach (Match mc in matches)
				skipStrings.Add((int)parser.Parse(mc.Groups["StringID"].Value,null));
			//String tables
			matches=strsRe.Matches(inString);
			Console.WriteLine("[General]");
			foreach (Match mc in matches)
			{
				Group strGrp=mc.Groups["String"], strIDGrp=mc.Groups["StringID"];
				for(int i=0;i<strIDGrp.Captures.Count;i++)
				{
					if(strGrp.Captures[i].Value!="")
					{
						int parsedVal=0;
						bool skip=false;
						try
						{
							parsedVal=(int)parser.Parse(strIDGrp.Captures[i].Value,null);
						}
						catch
						{
							skip=true;
						}
						skip|=skipStrings.Contains(parsedVal);
						if(!skip)
						{
							Console.Write(parsedVal);
							Console.Write("=");
							Console.WriteLine(strGrp.Captures[i].Value.Replace("\"\"","\""));
						}
					}
				}
			}
			//Control captions
			matches=ctrlsRe.Matches(inString);
			foreach (Match mc in matches)
			{
				Console.WriteLine("[" + mc.Groups["DialogID"].Value + "]");
				Console.WriteLine("0=" + mc.Groups["DialogCaption"].Value.Replace("\"\"","\""));
				Group captionGrp=mc.Groups["ControlCaption"], ctrlIDGrp=mc.Groups["ControlID"];
				for(int i=0;i<captionGrp.Captures.Count;i++)
				{
					if(captionGrp.Captures[i].Value!="")
					{
						Console.Write(ctrlIDGrp.Captures[i].Value);
						Console.Write("=");
						Console.WriteLine(captionGrp.Captures[i].Value.Replace("\"\"","\""));
					}
				}
			}
			Console.ReadLine();
			return 0;
		}
		private static string GetAssemblyAbout(System.Reflection.Assembly InAssembly)
		{
			if(InAssembly==null)
				InAssembly=System.Reflection.Assembly.GetExecutingAssembly();
			return InAssembly.GetName().Name + " by " + ((System.Reflection.AssemblyCompanyAttribute)InAssembly.GetCustomAttributes(typeof(System.Reflection.AssemblyCompanyAttribute), false)[0]).Company + " - v. " + InAssembly.GetName().Version.ToString();
		}
	}
}
