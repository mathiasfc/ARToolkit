#ifdef _WIN32
#include <windows.h>
#include <VideoIM.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <stdbool.h>

/* set up the video format globals */

#ifdef _WIN32
char			*vconf = "Data\\WDM_camera_flipV.xml";
#else
char			*vconf = "";
#endif

int             xsize, ysize;
int             thresh = 100;
int             count = 0;

char           *cparam_name    = "Data/camera_para.dat";
ARParam         cparam;

char                *config_name = "Data/multi/marker.dat";
ARMultiMarkerInfoT  *config;
ARMultiMarkerInfoT  ref;


struct ItemDaSala {
   int id;
   float posx,posy,posz;
   bool solo;
   bool visible;

};

struct ItemDaSala itensNaSala[5];
ARMultiEachMarkerInfoT marcadorReferencia;
ARMultiEachMarkerInfoT marcadorParaReposicionar;
ARMultiEachMarkerInfoT marcadoresVisiveis[10];

static void   init(void);
static void   cleanup(void);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static void   draw( double trans1[3][4], double trans2[3][4], int mode );
static void   DesenhaObjetos( double trans1[3][4], double trans2[3][4] );

float i0x;
float i0y;
float i1x;
float i1y;
float i2x;
float i2y;
float i3x;
float i3y;
float i4x;
float i4y;
int objectStyle = 0;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    init();

    arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
    return (0);
}



static void keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1c ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        cleanup();
        exit(0);
    }

    if( key == 't' ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        printf("Enter new threshold value (current = %d): ", thresh);
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
        count = 0;
    }

    /* turn on and off the debug mode with right mouse */
    if( key == 'q' ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        arDebug = 1 - arDebug;
        if( arDebug == 0 ) {
            glClearColor( 0.0, 0.0, 0.0, 0.0 );
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
        }
        count = 0;
    }

    /* turn on and off the debug mode with right mouse */
    if( key == 'e' ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        arDebug = 1 - arDebug;
        if( arDebug == 0 ) {
            glClearColor( 0.0, 0.0, 0.0, 0.0 );
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
        }
        count = 0;
    }

    if( key == 'n' ) {
        //NEW OBJECT
        int op;
        printf("\nAdicionar no solo(1) ou na parede(2)?");
        scanf ("%d",&op);
        if(op == 1){
            //add object solo
            AdicionaObjetoNoSolo();

        }else if(op == 2){
            printf("Parede esquerda(1) ou direita(2) ?");
            scanf ("%d",&op);
            if( op == 1){
                    AdicionaObjetoNaParede(true);
            }else if(op == 2){
                    AdicionaObjetoNaParede(false);
            }else{
                printf("\nInforme um valor váido.");
            }
        }else{
        printf("\nInforme um valor válido.");
        }
    }

    if( key == 'p' ) {
        RepositionObject();
    }

    //UPKEY
    if( key == 'w' ) {
        //gy += 2;
        //glTranslatef( 0.0, gy, 25.0 );
    }
    //LEFTKEY
    if( key == 'a' ) {
        //gx -= 2;
        //glTranslatef( gx, 0.0, 25.0 );
    }
    //DOWNKEY
    if( key == 's' ) {
        //gy -= 2;
        //glTranslatef( 0.0, gy, 25.0 );
    }
    //RIGHTKEY
    if( key == 'd' ) {
        //gx += 2;
        //glTranslatef( gx, 0.0, 25.0 );
    }

}

/* main loop */
static void mainLoop(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    double          err;
    int             i;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    /* detect the markers in the video frame */
    if( arDetectMarkerLite(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }

    argDrawMode2D();
    if( !arDebug ) {
        argDispImage( dataPtr, 0,0 );
    }
    else {
        argDispImage( dataPtr, 1, 1 );
        if( arImageProcMode == AR_IMAGE_PROC_IN_HALF )
            argDispHalfImage( arImage, 0, 0 );
        else
            argDispImage( arImage, 0, 0);

        glColor3f( 1.0, 0.0, 0.0 );
        glLineWidth( 1.0 );
        for( i = 0; i < marker_num; i++ ) {
            argDrawSquare( marker_info[i].vertex, 0, 0 );
        }
        glLineWidth( 1.0 );
    }

    arVideoCapNext();

    if( (err=arMultiGetTransMat(marker_info, marker_num, config)) < 0 ) {
        argSwapBuffers();
        return;
    }
    //printf("err = %f\n", err);
    if(err > 100.0 ) {
        argSwapBuffers();
        return;
    }
/*
    for(i=0;i<3;i++) {
        for(j=0;j<4;j++) printf("%10.5f ", config->trans[i][j]);
        printf("\n");
    }
    printf("\n");
*/
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    //for(i = 0; i < sizeof(itensNaSala) / sizeof(struct ItemDaSala); i++)
    //{
    //    printf(itensNaSala[i].id);
    //    printf("\n");
    //}

    for( i = 0; i < config->marker_num; i++ ) {
        if( config->marker[i].visible >= 0 ){
                marcadorReferencia = config->marker[i];
          //draw( config->trans, config->marker[i].trans, 0 );

        }
        else{
          //draw( config->trans, config->marker[i].trans, 1 );
        }
        //draw(config->trans, config->marker[i].trans, 1);
    }
    //DesenhaItens(config->trans, marcadorReferencia.trans,1);

    drawObjectsWithMarker(marcadorReferencia);

    argSwapBuffers();
}

static void init( void )
{
    ARParam  wparam;
    InicializaItensDaSala();
    /* open the video path */
    if( arVideoOpen( vconf ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (config = arMultiReadConfigFile(config_name)) == NULL ) {
        printf("config data load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 2, 1, 0 );
    arFittingMode   = AR_FITTING_TO_IDEAL;
    arImageProcMode = AR_IMAGE_PROC_IN_HALF;
    argDrawMode     = AR_DRAW_BY_TEXTURE_MAPPING;
    argTexmapMode   = AR_DRAW_TEXTURE_HALF_IMAGE;
}

/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

static void draw( double trans1[3][4], double trans2[3][4], int mode )
{
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_ambient1[]    = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash1[]      = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   mat_flash_shiny1[]= {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /* load the camera transformation matrix */
    glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd( gl_para );
    argConvGlpara(trans2, gl_para);
    glMultMatrixd( gl_para );

    if( mode == 0 ) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    }
    else {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash1);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny1);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
    }
    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 0, 0, 25.0 );
    glEnable(GL_COLOR_MATERIAL);

    glutSolidCube(50.0);
    //glColor3f(0.0,0.0,1.0);
    //if( !arDebug ) glutSolidCube(50.0);
     //else          glutWireCube(50.0);

    //glTranslatef( 3.0, 13.0, 35.0 );
     //glutSolidCone(20,20,20,20);
    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}

/* Desenha os objetos de acordo com o marcador de referencia */
void drawObjectsWithMarker(ARMultiEachMarkerInfoT marcador)
{
    //double position3d(*)[3] = marcador.pos3d;

    //A
    if(marcador.patt_id == 0){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //B
    }else if(marcador.patt_id == 1){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //C
    }else if(marcador.patt_id == 2){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //D
    }else if(marcador.patt_id == 3){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //G
    }else if(marcador.patt_id == 4){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //F
    }else if(marcador.patt_id == 5){
        //printf("%d", marcador.patt_id);
        //draw( config->trans, marcador.trans, 0 );
        DesenhaObjetos(config->trans, marcador.trans);
    //HIRO
    }else if(marcador.patt_id == 6){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    //KANJi
    }else if(marcador.patt_id == 7){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    }else if(marcador.patt_id == 8){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    }else if(marcador.patt_id == 9){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    }else if(marcador.patt_id == 10){
        //printf("%d", marcador.patt_id);
        draw( config->trans, marcador.trans, 0 );
    }
}

void RepositionObject(){
    int contaElementosValidos = 0;
    int i;
    memset(marcadoresVisiveis, 0, sizeof marcadoresVisiveis);
    for (i = 0; i < config->marker_num; i++) {
        if( config->marker[i].visible >= 0 ){
            marcadoresVisiveis[i] = config->marker[i];
            glColor3f(0.0,0.0,1.0);
            contaElementosValidos++;
        }
    }
    //int j;
    //for (j=0;j<contaElementosValidos;j++){
    //    marcadorParaReposicionar = marcadoresVisiveis[j]
    //}

    //printf("%d",contaElementosValidos);


}

void AdicionaObjetoNoSolo(){
    //printf("add obj solo");
    //itensNaSala
    int i;
    int objToAdd;
    for(i = 0; i < sizeof(itensNaSala) / sizeof(struct ItemDaSala); i++)
    {
        if(itensNaSala[i].visible == false){
            itensNaSala[i].visible = true;
        }
        printf(itensNaSala[i].id);
        printf("\n");
    }

}

void AdicionaObjetoNaParede(bool esq){
    if(esq){
        //printf("add obj parede esquerda");
    }else {
        //printf("add obj parede direita");
    }
}

void InicializaItensDaSala(){
    struct ItemDaSala item0;
    struct ItemDaSala item1;
    struct ItemDaSala item2;
    struct ItemDaSala item3;
    struct ItemDaSala item4;
    item0.id = "i0";
    item1.id = "i1";
    item2.id = "i2";
    item3.id = "i3";
    item4.id = "i4";

    item0.visible = false;
    item1.visible = false;
    item2.visible = false;
    item3.visible = false;
    item4.visible = false;


    itensNaSala[0] = item0;
    itensNaSala[1] = item1;
    itensNaSala[2] = item2;
    itensNaSala[3] = item3;
    itensNaSala[4] = item4;




}


//marcadorReferencia
//draw( config->trans, config->marker[i].trans, 1 );
static void DesenhaObjetos(double trans1[3][4], double trans2[3][4]){
    double    gl_para[16];
    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_ambient1[]    = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash1[]      = {1.0, 0.0, 0.0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   mat_flash_shiny1[]= {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd( gl_para );
    argConvGlpara(trans2, gl_para);
    glMultMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash1);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny1);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 0, 0, 25.0 );
    glEnable(GL_COLOR_MATERIAL);
    glColor3b(200.0,0.0,0.0);

    glutSolidCube(40.0);


}
















