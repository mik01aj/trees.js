
// most of the code here was translated from python module random

// 4 * _exp(-0.5)/_sqrt(2.0)
var NV_MAGICCONST = 1.7155277699214135;

var seed_x = 0, seed_y = 0, seed_z = 0;

// Normal distribution.
// mu = mean, sigma = standard deviation
function normalvariate(mu, sigma) {
    // Uses Kinderman and Monahan method. Reference: Kinderman,
    // A.J. and Monahan, J.F., "Computer generation of random
    // variables using the ratio of uniform deviates", ACM Trans
    // Math Software, 3, (1977), pp257-260.
    var u1, u2, z, zz;

    do {
        u1 = rand_uniform();
        u2 = 1.0 - rand_uniform();
        z = NV_MAGICCONST * (u1 - 0.5) / u2;
        zz = z * z / 4.0;
    }
    while (zz > -Math.log(u2));

    return mu + z * sigma;
}

function normalvariate2(params) {
    return normalvariate(params.mean, params.var);
}


//  Get the next random number in the range [0.0, 1.0).
function rand_uniform() {
    console.assert(seed_x != 0);
    console.assert(seed_y != 0);
    console.assert(seed_z != 0);
    // Wichman-Hill random number generator.
    //
    // Wichmann, B. A. & Hill, I. D. (1982)
    // Algorithm AS 183:
    // An efficient and portable pseudo-random number generator
    // Applied Statistics 31 (1982) 188-190
    //
    // see also:
    //        Correction to Algorithm AS 183
    //        Applied Statistics 33 (1984) 123
    //
    //        McLeod, A. I. (1985)
    //        A remark on Algorithm AS 183
    //        Applied Statistics 34 (1985),198-200

    // This part is thread-unsafe:
    // BEGIN CRITICAL SECTION
    seed_x = (171 * seed_x) % 30269;
    seed_y = (172 * seed_y) % 30307;
    seed_z = (170 * seed_z) % 30323;
    // END CRITICAL SECTION

    // Note: on a platform using IEEE-754 double arithmetic, this can
    // never return 0.0 (asserted by Tim; proof too long for a comment).
    return (seed_x / 30269.0 + seed_y / 30307.0 + seed_z / 30323.0) % 1.0;
}

// Initialize internal state. Distinct values between
// 0 and 27814431486575L inclusive are guaranteed to yield distinct
// internal states (this guarantee is specific to the default
// Wichmann-Hill generator).
function seed(s) {
    seed_x = s % 30268 + 1;
    s /= 30268;
    seed_y = s % 30306 + 1;
    s /= 30306;
    seed_z = s % 30322 + 1;
}


function chseed(s) {
    seed_y = (seed_y + s) % 30306 + 1;
    s /= 30306;
    seed_z = (seed_z + s) % 30322 + 1;
    s /= 30322;
    seed_x = (seed_x + s) % 30268 + 1;
}


var tree_params_p = {};
var ctx;
var tree_seed = 12345;

function drawing_init() {
    tree_params_p.height = 0.4;
    tree_params_p.trunk_width = 0;
    tree_params_p.trunk_height = 0.1;
    tree_params_p.width = 0.3;
    tree_params_p.angles = 0.5;
    tree_params_p.angles_var = 0;
    tree_params_p.angle_up = 0;
    tree_params_p.angle_down = 0;
    tree_params_p.angle_hor = 0;
    tree_params_p.fork = 0.1;
    tree_params_p.fork_var = 0.05;
    tree_params_p.straight_prob = 0;
    tree_params_p.branch_angle_correlation = 0.5;
    tree_params_p.branch_fork_correlation = 0;
    tree_params_p.branch_endlen_correlation = 0;
}

function render_frame() {
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, 500, 500); // TODO w, h

//   draw_tree(tree_seed-10000, 25, 95, 0, 1.0);
    draw_tree(tree_seed, 250, 490, 0, 1.0);
//   draw_tree(tree_seed+10000, 75, 95, 0, 1.0);
}

function interpolate(a, b, alpha) {
    return a * (1 - alpha) + b * alpha;
}

function draw_tree(r_seed, x, y, angle, width) {
    var ay, ax, len, w, fork_proportion;
    var i;
    var my_branch, c1, c2;

    console.assert(width > 0);

    seed(r_seed);

    len = 300 * tree_params_p.height
            * interpolate(0.05 + width, -3 * width * (width - 1),
            tree_params_p.branch_endlen_correlation);

    if (width == 1) {
        len = 1000 * tree_params_p.height * tree_params_p.trunk_height;
    }
    w = width * tree_params_p.width * 10;

    ay = Math.sin(angle);
    ax = -Math.cos(angle);
    {
        var w2 = (width == 1 ? w + tree_params_p.trunk_width * 10 : w);
        my_branch = {
            left: {
                x: x + ax * w2,
                y: y - ay * w2,
            },
            right: {
                x: x - ax * w2,
                y: y + ay * w2,
            }
        };
    }

    if (width < 0.001)
        return my_branch;

    // moving x and y to new position
    x += ay * len;
    y += ax * len;

    {
        var mean = tree_params_p.fork * 5
                * ((rand_uniform() > 0.5) ? 1 : -1);
        var variance = tree_params_p.fork_var * 5;
        var mult = interpolate(1, width, tree_params_p.branch_fork_correlation);
        fork_proportion = Math.atan(normalvariate(mean, variance) * mult) / Math.PI + 0.5;
    }

    var straight = (rand_uniform() < tree_params_p.straight_prob);

    for (i = 0; i < 2; i++) {
        var side = straight ? 0 : i * 2 - 1; // 1 or -1
        var other_branch_p = straight ? 0.1 :
                (side == 1 ? fork_proportion : 1 - fork_proportion);
        var new_width = width * (1 - other_branch_p);
        var angle_mult = Math.pow(1 - new_width, Math.pow(tree_params_p.branch_angle_correlation, 3) * 10);
        var angle_mean = side * tree_params_p.angles;
        var angle_diff = normalvariate(angle_mean,
                tree_params_p.angles_var);
        var new_angle = angle + Math.atan(angle_mult * angle_diff);
        var new_seed;

        new_angle += tree_params_p.angle_down * Math.sin(new_angle) * Math.pow(1 - width, 5);
        new_angle -= tree_params_p.angle_up * Math.sin(new_angle) * Math.pow(1 - width, 10);
        new_angle += tree_params_p.angle_hor * Math.sin(new_angle * 2) * Math.pow(1 - width, 15);

        seed(r_seed);
        chseed(i * 12345);
        new_seed = rand_uniform();

        // recursion!
        if (i == 0) {
            c1 = draw_tree(new_seed * 1000000,
                    x + ax * w * other_branch_p,
                    y - ay * w * other_branch_p,
                    new_angle,
                    new_width);
            if (straight)
                break;
        }
        else
            c2 = draw_tree(new_seed * 1000000,
                    x - ax * w * other_branch_p,
                    y + ay * w * other_branch_p,
                    new_angle,
                    new_width);
    }

    //glBegin(w > 0.08 ? GL_POLYGON : GL_LINE_LOOP);
    ctx.beginPath();
    ctx.moveTo(my_branch.right.x, my_branch.right.y);
    ctx.lineTo(my_branch.left.x, my_branch.left.y);
    ctx.lineTo(c1.left.x, c1.left.y);
    ctx.lineTo(c1.right.x, c1.right.y);
    if (!straight) {
        ctx.lineTo(c2.left.x, c2.left.y);
        ctx.lineTo(c2.right.x, c2.right.y);
    }
    ctx.closePath();

    ctx.fillStyle = 'white';
    ctx.strokeStyle = 'white';
    ctx.lineWidth = 0.2;
    ctx.fill();
    ctx.stroke();

    return my_branch;
}

// http://stackoverflow.com/a/11810957/115493
var checkLoad = function() {
    document.readyState !== "complete" ? setTimeout(checkLoad, 11) : onDomReady();
};


function onDomReady() {
    ctx = $('canvas')[0].getContext('2d');
    drawing_init();
    render_frame();

    var $body = $('body');

    var $sliders = $('#sliders');

    for (key in tree_params_p) { // TODO better iteration
        console.log(key);
        var newSlider = $('<div class="slider" data-param="' + key + '"><span class="state"></span></div>');
        newSlider.children('.state').css({left: tree_params_p[key] * 100 + '%'});
        $sliders.append(newSlider);
    }

    $('.slider').on('mousedown', function(ev1) {
        var currentSlider = ev1.target;
        $body.css({'-webkit-user-select': 'none'});
        $body.on('mousemove', function (ev2) {
            var clickedValue = (ev2.pageX - currentSlider.offsetLeft) / currentSlider.offsetWidth;
            clickedValue = Math.max(0, Math.min(clickedValue, 1));
            $(currentSlider).children('.state').css({left: clickedValue * 100 + '%'});
            tree_params_p[currentSlider.getAttribute('data-param')] = clickedValue;
            render_frame();
        });
        $body.one('mouseup', function () {
            $body.off('mousemove');
            $body.css({'-webkit-user-select': ''});
        });
    });
}

checkLoad();
