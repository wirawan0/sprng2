/*--- Chris S.: June 1999  */
/*--- reads in, first an integer as a generator type and next elements from */
/*--- a data file */
/*--- checking Default C interface                                        ---*/
/*--- added 'int gtype' 'scanf("%d\n",  @gtype)' and gtype to all init_sprng */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#define USE_MPI			 Uncomment to test with MPI */

#ifdef USE_MPI
#include <mpi.h>
#endif
#include "sprng.h"

#ifdef VERBOSE
#define report printf
#else
#define report ignore
#endif

#define PARAM SPRNG_DEFAULT

#define YES 1
#define NO 0

#define diff(a,b) (((a)>(b))?((a)-(b)):((b)-(a)))

int gtype; /*--- adding generator type ---*/ 

#ifdef __STDC__
void ignore(char *s, ...)
#else
void ignore(s)
char *s;
#endif
{
}


int check_gen()			/* Check generator with correct parameters   */
{
  int *gen1, *gen2, *gen3, *gen4, **gen5, **newgen1, **newgen2, i, size;
  char *s;
  int tempi, tempi2, correct, result = YES;
  int ngens, seed, nsp;
  float tempf1, tempf2;
  double tempd1;
  
  ngens = 3;
  seed = 985456376;
  gen1 = init_sprng(gtype,0,ngens,seed,PARAM); /* initiallize generators     */
  gen2 = init_sprng(gtype,1,ngens,seed,PARAM);
  gen3 = init_sprng(gtype,2,ngens,seed,PARAM);
  
  /* ____________________ Check arithmetic ___________________________       */

#ifdef CREATE_DATA
  for(i=0; i<500; i++)		/* generate integers                         */
    printf("%d\n", isprng(gen1));

  for(i=0; i<500; i++)		/* generate floats                           */
    printf("%8.6f\n", get_rn_flt(gen1));

  for(i=0; i<500; i++)
    printf("%16.14f\n", sprng(gen1)); /* generate double precision numbers   */

#else
  correct = YES;
  for(i=0; i<500; i++)		/* check integer arithmetic                  */
  {
    tempi2 = isprng(gen1);
    scanf("%d\n", &tempi);
    if(tempi != tempi2)
    {
      printf("%d. %d, %d\n", i, tempi, tempi2);
      result = correct = NO;
    }
  }
  
  if(correct == NO)
    printf("FAILED: Integer generator does not reproduce correct stream.\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Integer generator passed the reproducibility test\n");

  correct = YES;
  for(i=0; i<500; i++)		/* check float arithmetic                    */
  {
    tempf1 = get_rn_flt(gen1);
    scanf("%f\n", &tempf2);
    if(diff(tempf1,tempf2) > 1.0e-6)
    {
      printf("%d. %8.6f %8.6f\n", i, tempf2, tempf1);
      
      result = correct = NO;
    }
  }
  if(correct == NO)
    printf("FAILED: Float generator does not reproduce correct stream\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Float generator passed the reproducibility test\n");

  correct = YES;
  for(i=0; i<500; i++)		/* check double precision arithmetic         */
  {
    double tempd2;
    
    scanf("%lf\n", &tempd1);
    tempd2 = sprng(gen1);
    if(diff(tempd1,tempd2)>1.0e-14)
    {
      printf("%d. %18.15f %18.15f\n", i, tempd1, tempd2);
      
      result = correct = NO;
    }
    
  }
  if(correct == NO)
    printf("FAILED: Double generator does not reproduce correct stream.\n\tArithmetic on this machine may not be compatible with this generator.\n");
  else
    report("PASSED: Double generator spawns correctly\n");
#endif


  /* ____________________ Check spawning ___________________                 */

  nsp = 0;
  nsp += spawn_sprng(gen2,2,&newgen1);	/* spawn new generators              */
  nsp += spawn_sprng(newgen1[1],2,&newgen2);
  
#ifdef CREATE_DATA
  for(i=0; i<50; i++)	      /* generate numbers from new stream            */
    printf("%d\n", isprng(newgen2[1]));
#else
  if(nsp != 4)		     /* check if spawn_sprng returned correct value  */
  {
    result = NO;
    printf("FAILED: Generator was unable to spawn\n");
  }
  
  correct = YES;
  for(i=0; i<50; i++)	     /* check new stream                             */
  {
    int irn;
    scanf("%d\n", &tempi);
    irn = isprng(newgen2[1]);
    if(tempi != irn)
    {
      printf("%d. %x %x\n", i, tempi, irn);
      result = correct = NO;
    }
    
  }
  if(correct == NO)
    printf("FAILED: Generator does not reproduce correct stream after spawning\n\tThis is probably an error in spawning the generators\n");
  else
    report("PASSED: Generator spawns correctly\n");
#endif


  /* _____________________ Pack and unpack generator ______________________  */

#ifdef CREATE_DATA
  for(i=0; i<50; i++)		/* generate from original stream             */
    printf("%d\n", isprng(newgen2[1]));
  spawn_sprng(newgen2[1],1,&gen5); /* spawn from original stream             */
  for(i=0; i<50; i++)
    printf("%d\n", isprng(gen5[0]));
#else
  size = pack_sprng(newgen2[1], &s); /* pack the original stream             */
  if(size == 0)			/* check if pack_sprng succeeded             */
  {
    result = NO;
    printf("FAILED: Generator was unable to pack\n");
  }
  
  gen4 = unpack_sprng(s);	/* unpack generator                          */
  correct = YES;
  for(i=0; i<50; i++)	        /* check if unpacked stream = original stream*/
  {
    scanf("%d\n", &tempi);
    if(tempi != isprng(gen4))
      result = correct = NO;
  }
 if(correct == NO)
    printf("FAILED: Generator does not reproduce correct stream after packing and unpacking\n\tThis is probably an error in packing/unpacking the generators\n");
  else
    report("PASSED: Generator packs and unpacks stream correctly\n");

  correct = YES;
  spawn_sprng(gen4,1,&gen5);	/* spawn from unpacked stream                */
  
  for(i=0; i<50; i++)		/* check if spawned stream is correct        */
  {
    scanf("%d\n", &tempi);
    if(tempi != isprng(gen5[0]))
      result = correct = NO;
  }
  if(correct == NO)
    printf("FAILED: Generator does not spawn correct stream after packing and unpacking\n\tThis is probably an error in packing/unpacking the generators\n");
  else
    report("PASSED: Generator packs and unpacks spawning information correctly\n");
#endif

  /* _______________ Free generators ___________________                     */

#ifndef CREATE_DATA
  report("Checking free_sprng for integer generator ...\n");
  nsp = free_sprng(gen1);
  nsp = free_sprng(gen2);
  nsp = free_sprng(gen3);
  if(nsp != 6)	   /* check if free rng returns # of available generators    */
  {
    result = NO;
    printf("FAILED: Free returns %d instead of %d\n", nsp,6);
  }
  nsp = free_sprng(gen4);
  nsp = free_sprng(gen5[0]);
  nsp = free_sprng(newgen1[0]);
  nsp = free_sprng(newgen1[1]);
  nsp = free_sprng(newgen2[0]);
  nsp = free_sprng(newgen2[1]);
  if(nsp != 0)
  {
    result = NO;
    printf("FAILED: Free returns %d instead of %d\n", nsp,0);
  }
#endif
 
#ifndef CREATE_DATA
  report("\n... Completed checking generator \n\n");
#endif

  return result;
}













     /* Check if generator meets specifications in handling errors           */
int check_errors()
{
  int *gen1, **gen2, i;
  int tempi, correct, result = YES;
  int seed, nsp, size;
  char s[MAX_PACKED_LENGTH];
  
  seed = 985456376;
  
  /* ___________ ngens incorrect in init_sprng _____________                 */

#ifdef CREATE_DATA
  gen1 = init_sprng(gtype,0,1,seed,PARAM); /* take ngens = 1                       */
  for(i=0; i<50; i++)
    printf("%d\n", isprng(gen1));
#else
  correct = YES;
  fprintf(stderr,"Expect SPRNG WARNING: ngens <= 0.\n");
  gen1 = init_sprng(gtype,0,0,seed,PARAM);
  for(i=0; i<50; i++)	 /* ngens should be reset to 1   */
  {
    scanf("%d\n", &tempi);
    if(tempi != isprng(gen1))
      result = correct = NO;
  }
  if(correct == NO)
    printf("FAILED: Generator does not produce expected stream when ngens is 0 during initialization.\n");
  else
    report("PASSED: Generator produces expected stream when ngens is 0 during initialization.\n");
  
  nsp = free_sprng(gen1);     /* check if only one stream had been produced  */
  if(nsp != 0)
  {
    result = NO;
    printf("FAILED: Generator produces %d streams instead of 1 when ngens is 0 during initialization.\n",nsp+1);
  }
  else
    report("PASSED: Generator produces the correct number of streams when ngens is 0 during initialization.\n");
#endif


  /* _______________ invalid range for gennum _______________                */

#ifndef CREATE_DATA
  correct = YES;
  fprintf(stderr,"Expect SPRNG ERROR: gennum not in range\n");
  gen1 = init_sprng(gtype,-1,1,seed,PARAM); /* negative gennum */
  if(gen1 != NULL)
  {
    free_sprng(gen1);
    result = correct = NO;
  }
  
  fprintf(stderr,"Expect SPRNG ERROR: gennum not in range\n");
  gen1 = init_sprng(gtype,2,1,seed,PARAM); /* gennum >= ngens */
  if(gen1 != NULL)
  {
    free_sprng(gen1);
    result = correct = NO;
  }
  
  if(correct == NO)
    printf("FAILED: Generator does not return NULL when gennum is incorrect during initialization.\n");
  else
    report("PASSED: Generator returns NULL when gennum is incorrect during initialization.\n");

#endif



  /* _______________ Invalid parameter ______________________________        */

#ifdef CREATE_DATA
  gen1 = init_sprng(gtype,0,1,seed,SPRNG_DEFAULT); /* use default parameter */
  for(i=0; i<50; i++)
    printf("%d\n", isprng(gen1));
#else
  correct = YES;
  fprintf(stderr,"Expect SPRNG WARNING: Invalid parameter\n");
  gen1 = init_sprng(gtype,0,1,seed,1<<30);
  for(i=0; i<50; i++)		/* check if default parameter is used ...    */
  {				/* ... when an invalid parameter is passed.  */
    scanf("%d\n", &tempi);
    if(tempi != isprng(gen1))
      result = correct = NO;
  }
#endif



  /* _____________________ Check spawn with invalid ngens _________________ */
#ifdef CREATE_DATA
  spawn_sprng(gen1,1,&gen2);	/* spawn one generator */
    for(i=0; i<50; i++)
    printf("%d\n", isprng(gen2[0]));
#else
  report("Checking spawn with incorrect nspawned\n");
  
  fprintf(stderr,"Expect SPRNG WARNING: nspawned <= 0.\n");
  nsp = spawn_sprng(gen1,0,&gen2);
  free_sprng(gen1);
  if(nsp != 1)			/* check if one generator was spawned */
  {
    result = NO;
    printf("FAILED: Spawn returned %d streams instead of 1 when nspawned was greater than permitted.\n", nsp);
  }
    
  
  for(i=0; i<50; i++)		/* check spawned stream */
  {
    scanf("%d\n", &tempi);
    if(tempi != isprng(gen2[0]))
      result = correct = NO;
  }
  
  free_sprng(gen2[0]);
  
  if(correct == NO)
    printf("FAILED: Generator does not spawn correct stream when nspawned was 0.\n");
  else
    report("PASSED: Generator spawns correctly when nspawned was 0.\n");
#endif


  /* ____________________ Unpack invalid string _____________________ */

#ifndef CREATE_DATA
  memset(s,0,MAX_PACKED_LENGTH); /* set string to 0's */

  
  fprintf(stderr,"Expect SPRNG ERROR: packed string invalid\n");
  gen1 = unpack_sprng(s);
  if(gen1 != NULL)	    /* NULL should be returned for invalid string */
  {
    result = NO;
    printf("FAILED: Generator unpacks invalid string\n");
  }
  else
    report("PASSED: Generator detected invalid string while unpacking\n");
#endif

  return result;
}



#ifdef USE_MPI
#ifdef __STDC__
int check_mpi_seed(unsigned int seed)
#else
int check_mpi_seed(seed)
unsigned int seed;
#endif
{
  int nprocs, myid, result = YES, i, tag=0;
  MPI_Status status;
  unsigned int temp;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if(myid != 0)
    MPI_Send(&seed, 1, MPI_UNSIGNED, 0, tag, MPI_COMM_WORLD);
  else
    for(i=1; i<nprocs; i++)
    {
      MPI_Recv(&temp,1, MPI_UNSIGNED, i, tag, MPI_COMM_WORLD, &status);
      if(temp != seed)
	result = NO;
    }
  
  if(result == NO)
    printf("FAILED: Seeds returned by make_seed differ on different processors\n");
  else
    report("PASSED: Seeds returned my make_seed on all processes are equal.\n");
  
  return result;
}
#endif




#ifdef __STDC__
int main(int argc, char *argv[])
#else
int main(argc, argv)
int argc;
char *argv[];
#endif
{
  int result=YES;
  
#ifndef CREATE_DATA
  int temp, myid;
  unsigned int seed1, seed2;
 
  report("Checking make_sprng_seed ...  ");

#ifdef USE_MPI
  MPI_Init(&argc, &argv);
#endif
  seed1 = make_sprng_seed();
#ifdef USE_MPI
  result = check_mpi_seed(seed1);
#endif

  seed2 = make_sprng_seed();
#ifdef USE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  if(myid==0)
  {
#endif
  if(seed1 != seed2)
    report("  ... Checked make_sprng_seed\n");
  else
  {
    result = NO;
    printf("\nERROR: make_sprng_seed does not return unique seeds\n");
  }
#endif
  scanf("%d\n", &gtype);  /* reading in a generator type */
  
  if(check_gen() != YES)
    result = NO;

  if(check_errors() != YES)
    result = NO;

#ifndef CREATE_DATA
  if(result == YES)
    printf("\nResult:\t PASSED\n\n");
  else
    printf("\nResult:\t FAILED\n\n");
#endif

#ifdef USE_MPI
  }
  MPI_Finalize();
#endif
  return 0;
}

