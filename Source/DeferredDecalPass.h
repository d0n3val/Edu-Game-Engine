#ifndef _DEFERRED_DECAL_PASS_H_
#define _DEFERRED_DECAL_PASS_H_

class Framebuffer;

class DeferredDecalPass
{
public:
    DeferredDecalPass();

    void execute(Framebuffer* target, uint width, uint height);

private:

};

#endif /* _DEFERRED_DECAL_PASS_H_ */
