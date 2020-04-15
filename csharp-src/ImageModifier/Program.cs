using System;
using System.IO;
using System.Linq;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Processing;
using SixLabors.ImageSharp.Primitives;

namespace ImageModifier
{
	class Program
	{
		static void Main(string[] args)
		{
			if(args.Length < 3)
			{
				string executable = new string(Environment.GetCommandLineArgs()[0].TakeLast(32).ToArray());
				if(executable.Length == 32)
					executable = "..." + executable;

				Console.WriteLine($"usage: {executable} <grey|emboss|blur|hsv> <input file> <output file>\n");
				Console.WriteLine("convert image colors\n\tgrey\tconverts the colors to greyscale\n\thsv\tconverts the rgba to the hsv colorspace\n");
				Console.WriteLine("apply filter to image\n\temboss\tapplies the emboss filter\n\tblur\tblurs the image via a gaussian blur filter\n");
				return;
			}

			FileInfo sourceFile = new FileInfo(args[1]);
			FileInfo destinationFile = new FileInfo(args[2]);

			if(!sourceFile.Exists)
			{
				Console.WriteLine("The file could not be found.");
				return;
			}

			using(Image image = Image.Load(sourceFile.FullName))
			{
				switch(args[0])
				{
					case "grey":
						image.Mutate(x => x.Grayscale());
						break;
					case "blur":
						image.Mutate(x => x.GaussianBlur());
						break;
					case "emboss":
						break;
					case "hsv":
						break;
					default:
						Console.WriteLine($"The operation {args[0]} is not available.");
						return;
				}

				image.Save(destinationFile.FullName);
			}
		}
	}
}
