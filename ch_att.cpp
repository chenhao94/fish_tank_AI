#include "fish.h"
#include "ch_att.h"
// version 1.0.att

static const double speedEstimate=1.2;
static const long SpeedCap=50;
static const long dir[4][2]={{1,0},{0,1},{-1,0},{0,-1}};

others_ch_att::others_ch_att():hp(0),dietime(0),sp(0),x(0),y(0),lastx(0),lasty(0),maxmove(1),status(0)
{}

void ch_att::getInfo()
{
	x=getX();
	y=getY();
	lev=getLevel();
	point=getPoint();
	exp=getExp();
	hp=getHP();
	maxhp=getMaxHP();
	att=getAtt();
	sp=getSp();
	getMap();
}

void ch_att::getMap() // Get all infos about the map and the players.
{
	for (int i=1;i<=MAX_PLAYER;i++)
	 if (i!=myID)
	  {
		p[i].status=DEAD;
		p[i].lastx=p[i].x;
		p[i].lasty=p[i].y;
		p[i].x=0;
		p[i].y=0;
		p[i].hp=askHP(i);
	  }
	for (int i=1,dis,id;i<=N;i++)
	 for (int j=1;j<=M;j++)
	  {
		g[i][j]=askWhat(i,j);
		if (g[i][j]>EMPTY && g[i][j]!=myID)
		 {
			id=g[i][j];
			p[id].x=i;
			p[id].y=j;
			p[id].status=ALIVE;
			if (p[id].lastx>0)
			 {
				dis=abs1(p[id].lastx-p[id].x)+abs1(p[id].lasty-p[id].y);
				p[id].maxmove=max1(p[id].maxmove,dis);
				p[id].sp=(long)(speedEstimate*p[id].maxmove);
			 }
		 }
	  }
	for (int i=1;i<=MAX_PLAYER;i++)
	 if (i!=myID && p[i].status==DEAD)
	  p[i].dietime++;
}

long ch_att::targetSpeed()
{
	long low=oo,high=-oo,sec_low=oo,sec_high=-oo;
	long speed=0,ans=0;
	for (int i=1;i<=MAX_PLAYER;i++)
	 if (i!=myID)
	  {
		speed=p[i].sp;
		if (speed>high)
		 {
			sec_high=high;
			high=speed;
		 }
		else if (speed>sec_high)
		 sec_high=speed;
		
		if (speed<low)
		 {
			sec_low=low;
			low=speed;
		 }
		else if (speed<sec_low)
		 sec_low=speed;
	  }
	ans=(long)(sec_low+(sec_high-sec_low)*0.618);
	return (long)(max1(ans,1+lev*0.5));
}

void ch_att::init()
{
	myID=getID();
	getInfo();
	for (int i=1;i<=5;i++)
	 increaseHealth();
	for (int i=1;i<=3;i++)
	 increaseStrength();
	for (int i=1;i<=2;i++)
	 increaseSpeed();
}

void ch_att::assignPoints()
{
	long est=0;
	
	// Health
	while (point>0 && hp<att*7/3)
	 {
		point--;
		increaseHealth();
	 }
	// Speed
	est=targetSpeed();
	while (point>0 && sp<SpeedCap && sp<=est)
	 {
		point--;
		increaseSpeed();
	 }
	// Strength
	while (point>0)
	 {
		point--;
		increaseStrength();
	 }
}

void ch_att::play()
{
	long r;
	double rate=(double)hp/(double)maxhp;
	char flag=!MOVED;
	srand(time(NULL));
	getInfo();
	assignPoints();
	
	//Try to attack
	if (rate<0.4)
	 {
		if (emergency())
		 flag=MOVED;
		else goto RUN;
	 }
	else if (rate<0.8 || lev<10)
	 {
		if (normalAtt())
		 flag=MOVED;
	 }
	else
	 {
		if (healthyAtt())
		 flag=MOVED;
	 }
	if (flag==MOVED)
	 goto ASSIGN_POINTS;
	
	r=rand()%10+1;
	if (r<=7 && multiHit(2))
	 flag=MOVED;
	else if (r<=3 && multiHit(3))
	 flag=MOVED;
	else if (r<=1 && multiHit(4))
	 flag=MOVED;
	
	//Run
	if (flag!=MOVED)
	{
	RUN:
	if (rate<0.4)
	 run(0.7);
	else 
	 run(0.3);
	}
	
	ASSIGN_POINTS: 
	getInfo();
	assignPoints();
}

bool ch_att::emergency()
{
	long xx,yy,tx,ty,mx=0,my=0,ax=0,ay=0,max=-oo,maxexp=0;
	long est=0,id,estexp=0,estlev=0;
	for (int i=0;i<=sp;i++)
	 for (int j=0;j<=i;j++)
	  for (int d1=-1;d1<=1;d1+=2)
	   for (int d2=-1;d2<=1;d2+=2)
	    {
			xx=x+j*d1;
			yy=y+(i-j)*d2;
			if (!legal(xx,yy) || (g[xx][yy]!=EMPTY && g[xx][yy]!=myID) )
			 continue;
			for (int k=0;k<4;k++)
			  {
				tx=xx+dir[k][0],ty=yy+dir[k][1];
				if (!legal(tx,ty) || !g[tx][ty] || g[tx][ty]==myID)
				 continue;
				estexp=1;
				if (g[tx][ty]==FOOD)
				 est=max1(2,maxhp/10);
				else
				 {
					id=g[tx][ty];
					if (att>=p[id].hp)
					 {
						estlev=estLevel(id);
						estexp=max1(1,estlev/2);
					 }
					else
					 continue;
					if (estexp>=levelUpRequire())
					 est=6;
					else
					 est=0;
				 }
				if (est>max || (est==max && estexp>maxexp))
				 {
					max=est;
					maxexp=estexp;
					mx=xx,my=yy;
					ax=tx,ay=ty;
				 }
			  }
		}
	if (!mx)
	 return 0;
	move(mx,my);
	attack(ax,ay);
	return 1;
}

bool ch_att::normalAtt()
{
	long xx,yy,tx,ty,mx=0,my=0,ax=0,ay=0,max=0,maxexp=-oo;
	long est=0,id,estexp=0,estlev=0;
	for (int i=0;i<=sp;i++)
	 for (int j=0;j<=i;j++)
	  for (int d1=-1;d1<=1;d1+=2)
	   for (int d2=-1;d2<=1;d2+=2)
	    {
			xx=x+j*d1;
			yy=y+(i-j)*d2;
			if (!legal(xx,yy) || (g[xx][yy]!=EMPTY && g[xx][yy]!=myID))
			 continue;
			for (int k=0;k<4;k++)
			  {
				tx=xx+dir[k][0],ty=yy+dir[k][1];
				if (!legal(tx,ty) || !g[tx][ty] || g[tx][ty]==myID)
				 continue;
				estexp=1;
				if (g[tx][ty]==FOOD)
				 est=max1(2,maxhp/10);
				else
				 {
					id=g[tx][ty];
					if (att>=p[id].hp)
					 {
						estlev=estLevel(id);
						estexp=max1(1,estlev/2);
					 }
					else
					 continue;
					if (estexp>=levelUpRequire())
					 est=6;
					else
					 est=0;
				 }
				if (estexp>maxexp || (estexp==maxexp && est>max))
				 {
					max=est;
					maxexp=estexp;
					mx=xx,my=yy;
					ax=tx,ay=ty;
				 }
			  }
		}
	if (!mx)
	 return 0;
	move(mx,my);
	attack(ax,ay);
	return 1;
}

bool ch_att::healthyAtt()
{
	long xx,yy,tx,ty,mx=0,my=0,ax=0,ay=0,max=0,maxexp=-oo;
	long est=0,id,estexp=0,estlev=0;
	for (int i=0;i<=sp;i++)
	 for (int j=0;j<=i;j++)
	  for (int d1=-1;d1<=1;d1+=2)
	   for (int d2=-1;d2<=1;d2+=2)
	    {
			xx=x+j*d1;
			yy=y+(i-j)*d2;
			if (!legal(xx,yy) || (g[xx][yy]!=EMPTY && g[xx][yy]!=myID))
			 continue;
			for (int k=0;k<4;k++)
			  {
				tx=xx+dir[k][0],ty=yy+dir[k][1];
				if (!legal(tx,ty) || g[tx][ty]<=0 || g[tx][ty]==myID)
				 continue;
				
				id=g[tx][ty];
				if (att>=p[id].hp)
				 {
					estlev=estLevel(id);
					estexp=max1(1,estlev/2);
				 }
				else
				 continue;
				if (estexp>=levelUpRequire())
				 est=6;
				else
				 est=0;
				
				if (estexp>maxexp || (estexp==maxexp && est>max))
				 {
					max=est;
					maxexp=estexp;
					mx=xx,my=yy;
					ax=tx,ay=ty;
				 }
			  }
		}
	if (!mx)
	 return 0;
	move(mx,my);
	attack(ax,ay);
	return 1;
}

bool ch_att::multiHit(long n)
{
	long xx,yy,tx,ty,mx=0,my=0,ax=0,ay=0,max=0,maxexp=-oo;
	long est=0,id,estexp=0,estlev=0;
	for (int i=0;i<=sp;i++)
	 for (int j=0;j<=i;j++)
	  for (int d1=-1;d1<=1;d1+=2)
	   for (int d2=-1;d2<=1;d2+=2)
	    {
			xx=x+j*d1;
			yy=y+(i-j)*d2;
			if (!legal(xx,yy) || (g[xx][yy]!=EMPTY && g[xx][yy]!=myID))
			 continue;
			for (int k=0;k<4;k++)
			  {
				tx=xx+dir[k][0],ty=yy+dir[k][1];
				if (!legal(tx,ty) || g[tx][ty]<=0 || g[tx][ty]==myID)
				 continue;
				
				id=g[tx][ty];
				if (att*n>=p[id].hp)
				 {
					estlev=estLevel(id);
					estexp=max1(1,estlev/2);
				 }
				else
				 continue;
				if (estexp>=levelUpRequire())
				 est=6;
				else
				 est=0;
				
				if (estexp>maxexp || (estexp==maxexp && est>max))
				 {
					max=est;
					maxexp=estexp;
					mx=xx,my=yy;
					ax=tx,ay=ty;
				 }
			  }
		}
	if (!mx)
	 return 0;
	move(mx,my);
	attack(ax,ay);
	return 1;
}

double ch_att::risk(long x,long y)
{
	double ans=0,rate;
	long id,estStrength=0,rest,dis;
	for (int i=1;i<=N;i++)
	 for (int j=1;j<=M;j++)
	  if (g[i][j]>0 && g[i][j]!=myID)
	   {
			id=g[i][j];
			estStrength=(long)(p[id].sp*1.2);
			rest=max1(0,hp-estStrength);
			rate=(double)rest/(double)maxhp;
			dis=abs1(i-x)+abs1(j-y);
			ans+=1/(rate+eps)/(rate+eps)*p[id].sp/dis;
	   }
	return ans;
}

double ch_att::benefit(long x,long y)
{
	double ans=0,rate;
	long id,rest,dis;
	for (int i=1;i<=N;i++)
	 for (int j=1;j<=M;j++)
	  if (g[i][j]>0 && g[i][j]!=myID)
	   {
			id=g[i][j];
			rest=max1(0,p[id].hp-att);
			rate=(double)rest/(double)maxhp;
			dis=abs1(i-x)+abs1(j-y);
			ans+=1/(rate+eps)/(rate+eps)*sp/dis*(1+log(estLevel(id)));
	   }
	  else if (g[i][j]==FOOD)
	   ans+=1/eps*sp/dis;
	return ans;
}

void ch_att::run(double riskRate)
{
	long xx,yy,mx=0,my=0;
	double max=-(1e100),value;
	for (int i=0;i<=sp;i++)
	 for (int j=0;j<=i;j++)
	  for (int d1=-1;d1<=1;d1+=2)
	   for (int d2=-1;d2<=1;d2+=2)
	    {
			xx=x+j*d1;
			yy=y+(i-j)*d2;
			if (!legal(xx,yy) || (g[xx][yy]!=EMPTY && g[xx][yy]!=myID))
			 continue;
			value=-riskRate*risk(xx,yy)+(1-riskRate)*benefit(xx,yy);
			if (value>max)
			 {
				mx=xx;
				my=yy;
			 }
		}
	move(mx,my);
}
