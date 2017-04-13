using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RAInterface
{
    public class ComparisonVariable
    {
        public enum Size
        {
            Bit_0,
            Bit_1,
            Bit_2,
            Bit_3,
            Bit_4,
            Bit_5,
            Bit_6,
            Bit_7,
            Bit_NibbleLower,
            Bit_NibbleUpper,
            Bit_8Bit,
            Bit_16Bit,
            Bit_32Bit,

            SizeTypes
        }

        public enum ComparisonVariableType
        {
            Address,            //  byte addressor
            Value,              //  quantity, assumed to be unsigned 32bit
            DeltaMem,           //  the previous value at this address
            DynamicVariable,    //  Unused
        }

        ComparisonVariable(Size sz, ComparisonVariableType type)
        {
            CompSize = sz;
            CompType = type;
        }

        public readonly Size CompSize;
        public readonly ComparisonVariableType CompType;
    }

    public class Condition
    {
        public enum ComparisonType
        {
            Equals,
            LessThan,
            LessThanOrEqualTo,
            GreaterThan,
            GreaterThanOrEqualTo,
            NotEqualTo,

            NumComparisonTypes
        }

        Condition(ComparisonVariable src, ComparisonType type, ComparisonVariable dest)
        {
            Source = src;
            CompType = type;
            Dest = dest;
        }

        public ComparisonVariable Source;
        public ComparisonType CompType;
        public ComparisonVariable Dest;
    }
}
