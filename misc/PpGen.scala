


class PpGen
{

val header = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Resampler Resampler;

typedef void (floatfunc)(void *ctx, float v);
typedef void (resamplerfunc)(Resampler *self, float v, floatfunc func, void *ctx);



struct Resampler
{
    int decimation;
    resamplerfunc *decimate;
    resamplerfunc *interpolate;
    float sum;
    float d0;
    float d1;
};

"""

val footer = """
Resampler *resamplerCreate(int decimation)
{
    Resampler *r = (Resampler *) malloc(sizeof(Resampler));
    if (!r)
        return NULL;
    memset(r, 0, sizeof(Resampler));
    r->decimation = decimation;
    switch (decimation)
        {
        case  2 : r->decimate = dec0200; r->interpolate = int02; break;
        case  3 : r->decimate = dec0300; r->interpolate = int03; break;
        case  4 : r->decimate = dec0400; r->interpolate = int04; break;
        case  5 : r->decimate = dec0500; r->interpolate = int05; break;
        case  6 : r->decimate = dec0600; r->interpolate = int06; break;
        case  7 : r->decimate = dec0700; r->interpolate = int07; break;
        case  8 : r->decimate = dec0800; r->interpolate = int08; break;
        case  9 : r->decimate = dec0900; r->interpolate = int09; break;
        case 10 : r->decimate = dec1000; r->interpolate = int10; break;
        default : free(r); return NULL;
        }
    return r;
}


void resamplerDelete(Resampler *r)
{
    if (r) free(r);
}
"""
    /**
     * Quick and easy low pass coefficients
     * Gave this class its own implementation for portability
     */
    private def lowPassCoeffs(decimation: Int, size: Int) : Array[Double] =
        {
        val twopi = 2.0 * math.Pi
        val omega = Math.Pi / decimation
        val bottom = -0.5 * size
        val xs = Array.tabulate(size)( idx =>
            {
            //FIR coefficient
            val i = bottom + idx
            val fir = if (i == 0)
                omega / math.Pi 
            else 
                math.sin(omega * i) / (math.Pi * i)
            //Hamming window
            val window = 0.54 - 0.46 * math.cos(twopi * idx / (size-1))
            fir * window
            })
        xs
        }






    def generate(decimation: Int, m: Int) =
        {
        val size = decimation * m
        val coeffs = lowPassCoeffs(decimation, size)
        println("\n\n\n")
        println("//####################################################################")
        println("//###  Decimation: " + decimation)
        println("//####################################################################")
        for (i <- 0 until coeffs.size) 
            {
            println("#define c%02d%02d %g".format(decimation, i, coeffs(i)))
            }
        println("")
          
        for (i <- 0 until decimation)
            {
            println("static void dec%02d%02d(Resampler *r, float v, floatfunc func, void *ctx);".format(decimation, i))
            }
        println("")
        for (i <- 0 until decimation)
            {
            println("static void dec%02d%02d(Resampler *r, float v, floatfunc func, void *ctx)".format(decimation, i))
            println("{")
            println("float d0 = r->d0; float d1 = r->d1;")
            if (i == 0)
                {
                println("r->sum = d0 * c%02d%02d + d1 * c%02d%02d + v * c%02d%02d;".
                    format(decimation,i,decimation,i+decimation,decimation, i+decimation+decimation))
                }
            else if (i == decimation-1)
                {
                println("func(ctx, r->sum + d0 * c%02d%02d + d1 * c%02d%02d + v * c%02d%02d);".
                    format(decimation,i,decimation,i+decimation,decimation, i+decimation+decimation))
                }
            else
                {
                println("r->sum += d0 * c%02d%02d + d1 * c%02d%02d + v * c%02d%02d;".
                    format(decimation,i,decimation,i+decimation,decimation, i+decimation+decimation))
                }
            println("r->d0 = d1; r->d1 = v;")
            println("r->decimate = dec%02d%02d;".format(decimation, (i+1)%decimation))
            println("}")
            println("")
            }
        println("static void int%02d(Resampler *r, float v, floatfunc func, void *ctx)".format(decimation))
        println("{")
        println("float d0 = r->d0; float d1 = r->d1;")
        for (i <- 0 until decimation)
            {
            println("func(ctx, d0 * c%02d%02d + d1 * c%02d%02d + v * c%02d%02d);".
                format(decimation, i, decimation, i+decimation, decimation, i+decimation+decimation))
            }
        println("r->d0 = d1; r->d1 = v;")
        println("}")
        println("")
        }









    def doit =
        {
        println(header)
        for (decimation <- 2 until 11)
            generate(decimation, 3)
        println(footer)
        }


}











object PpGen
{
     def main(argv:  Array[String]) : Unit =
         {
         val obj = new PpGen
         obj.doit
         }
}

