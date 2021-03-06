﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace TestCodeGen
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            var testCase = new TestCase("Byte converter test");
            var initCodes = new List<string>
            {
                "auto filename = \"byteConverterTest\";",
                "auto path = MakeFilePath(filename);",
                "auto writer = FileWriter(path, 0);",
                "auto reader = FileReader(path, 0);",
            };
            testCase.AddCode(initCodes);
            var testFile =
                new TestFile("../../TestFrame/FlyTest.hpp", "byteConverterTest_gen",
                    testCase);


            var types = TypeFactory.GenBasicType().ConvertAll(basic => (IType)basic);
            var specialization = types.GenSpecializationType();
            //specialization.ForEach(e => Console.WriteLine(e.Name));
            types.AddRange(specialization);
            var compoundTypes = types.GenCompoundType(2).ToArray();
            types.AddRange(compoundTypes);

            var i = 0;
            types.ForEach(x => testCase.Add(
                new Section("section" + (i++),
                    new CodeSnippet_ByteConverter(x))));

            using (var writer = new StreamWriter("ByteConvereterTest_Gen.cpp"))
            {
                writer.Write(testFile.ToString());
            }
            //Console.Write();
        }
    }
}
